#include "ndi_mod.h"
#include "ndi_mod_audio.h"

#include <iostream>
#include <map>

// lua
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

extern "C" {
// matron
#include "lua_eval.h"
#include "hardware/screen.h"
// local copies of private weaver types/methods
#include <weaver_image.h>
}

#define MSG(contents) \
   std::cerr << "ndi-mod: " << contents << "\n"

SenderRecord::SenderRecord()
: surface(NULL), sender(NULL), sender_name(""), buffer(NULL), buffer_size(0)
{}

SenderRecord::SenderRecord(SenderRecord&& other) {
    surface = other.surface;
    sender = other.sender;
    sender_name = other.sender_name;
    buffer = other.buffer;
    buffer_size = other.buffer_size;
    frame = other.frame;
    frame_divisor = other.frame_divisor;
    frame_counter = other.frame_counter;

    other.surface = NULL;
    other.sender_name = "";
    other.sender = NULL;
    other.buffer = NULL;
    other.buffer_size = 0;
}

SenderRecord::SenderRecord(cairo_surface_t* surface, const char* name)
: surface(surface), sender_name(name), buffer(NULL), buffer_size(0)
{
    NDIlib_send_create_t send_create;
    send_create.p_ndi_name = name;
    send_create.p_groups = NULL;
    send_create.clock_video = false;
    send_create.clock_audio = false;

    sender = NDIlib_send_create(&send_create);
    if (!sender) {
        MSG("error creating NDI sender");
    }

    frame_divisor = 1;
    frame_counter = 0;

    // NDI video format: 60fps RGBA progressive (with alpha ignored.)
    // norns cairo surfaces are CAIRO_FORMAT_ARGB32 (premultiplied ARGB.)
    // But all four bytes are always the same, so the RGBA/ARGB mismatch
    // doesn't matter, and we can use the surface data directly.
    frame = {
        .frame_rate_N = 60000,
        .frame_rate_D = frame_divisor * 1000,
        .FourCC = NDIlib_FourCC_type_RGBX,
        .frame_format_type = NDIlib_frame_format_type_progressive
    };

    cairo_surface_reference(surface); // increase refcount
    MSG("NDI sender \"" << sender_name << "\" created");
}

SenderRecord::~SenderRecord() {
    if (sender != NULL) {
        NDIlib_send_destroy(sender);
        free(buffer);
        MSG("NDI sender destroyed");
    }
    if (surface != NULL) {
        cairo_surface_destroy(surface); // decrease refcount
    }
}

std::map<cairo_surface_t*, SenderRecord> surface_sender_map;

bool running = false;
static bool initialized = false;
static bool failed = false;

//
// core functions
//

int create_sender(cairo_surface_t* surface, const char* name) {
    auto it = surface_sender_map.find(surface);
    if (it != surface_sender_map.end()) {
        MSG("an NDI sender already exists for this surface");
        return 0;
    }

    surface_sender_map.emplace(surface, SenderRecord{surface, name});

    return 0;
}

int destroy_sender(cairo_surface_t* surface) {
    auto it = surface_sender_map.find(surface);
    if (it == surface_sender_map.end()) {
        MSG("No NDI sender exists for this surface");
        return 0;
    }

    surface_sender_map.erase(it);
    return 0;
}

int initialize_ndi() {
    if (!initialized && !failed) {
        if (!NDIlib_initialize()) {
            MSG("Error initializing NDI library");
            failed = true;
            return 0;
        }

        MSG("NDI service initialized");
        initialized = true;

        // create the default sender
        cairo_t* ctx = (cairo_t*)screen_context_get_primary();
        if (ctx == NULL) {
            return 0;
        }

        cairo_surface_t* surface = cairo_get_target(ctx);
        create_sender(surface, "screen");
    }
    return 0;
}

int cleanup_ndi() {
    running = false;
    cleanup_jack();
    if (initialized) {
        initialized = false;
        surface_sender_map.clear();
        NDIlib_destroy();
        MSG("NDI service stopped");
    }
    return 0;
}

void SenderRecord::send()
{
    if (cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE ||
        cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
        return;
    }

    frame_counter++;
    if (frame_counter % frame_divisor != 0) {
        return;
    } else {
        frame_counter = 0;
    }
    frame.frame_rate_D = frame_divisor * 1000;

    // prepare the frame and send it
    unsigned char* data = cairo_image_surface_get_data(surface);
    if (data != NULL) {
        frame.xres = cairo_image_surface_get_width(surface);
        frame.yres = cairo_image_surface_get_height(surface);
        frame.line_stride_in_bytes = cairo_image_surface_get_stride(surface);

        size_t bytes = frame.yres * frame.line_stride_in_bytes;
        if (buffer_size < bytes) {
            MSG("Reallocating NDI framebuffer for \"" << sender_name << "\"");
            buffer = (unsigned char*)realloc(buffer, bytes);
            buffer_size = bytes;
        }

        memcpy(buffer, data, bytes);
        frame.p_data = buffer;

        NDIlib_send_send_video_async_v2(sender, &frame);
    }
}

static void update_all_surfaces()
{
    if (initialized && running)
    {
        for (auto& pair : surface_sender_map) {
            if (!failed) {
                pair.second.send();
            }
        }
    }
}

//
// lua method implementations
//

static int ndi_mod_init(lua_State *l) {
    lua_check_num_args(0);
    return initialize_ndi();
}

static int ndi_mod_cleanup(lua_State *l) {
    lua_check_num_args(0);
    return cleanup_ndi();
}

static int ndi_mod_update(lua_State *l) {
    lua_check_num_args(0);
    update_all_surfaces();
    return 0;
}


static int ndi_mod_start(lua_State *l) {
    lua_check_num_args(0);
    running = true;
    return 0;
}

static int ndi_mod_stop(lua_State *l) {
    lua_check_num_args(0);
    running = false;
    return 0;
}

static int ndi_mod_is_running(lua_State *l) {
    lua_check_num_args(0);
    lua_pushboolean(l, running);
    return 1;
}

static int ndi_mod_init_audio(lua_State *l) {
    int nargs = lua_gettop(l);

    // check if first arg is a table (multi-channel mode)
    if (nargs >= 1 && lua_istable(l, 1)) {
        // multi-channel mode: extract port names from table
        int num_channels = lua_rawlen(l, 1);
        if (num_channels <= 0) {
            return luaL_error(l, "init_audio: table must contain at least one port name");
        }

        const char** ports = new const char*[num_channels];
        for (int i = 0; i < num_channels; i++) {
            lua_rawgeti(l, 1, i + 1);
            if (!lua_isstring(l, -1)) {
                delete[] ports;
                return luaL_error(l, "init_audio: all table elements must be strings");
            }
            ports[i] = lua_tostring(l, -1);
            lua_pop(l, 1);
        }

        initialize_jack(ports, num_channels);
        delete[] ports;
    } else {
        // stereo mode (backward compatible)
        const char* output_left = "crone:output_1";
        const char* output_right = "crone:output_2";

        if (nargs >= 1 && lua_isstring(l, 1)) {
            output_left = lua_tostring(l, 1);
        }
        if (nargs >= 2 && lua_isstring(l, 2)) {
            output_right = lua_tostring(l, 2);
        }

        const char* ports[2] = { output_left, output_right };
        initialize_jack(ports, 2);
    }

    return 0;
}

static int ndi_mod_cleanup_audio(lua_State *l) {
    lua_check_num_args(0);
    cleanup_jack();
    return 0;
}

static int ndi_mod_create_image_sender(lua_State *l) {
    lua_check_num_args(2);
    _image_t *i = _image_check(l, 1);
    const char *name = luaL_checkstring(l, 2);

    if (i->surface != NULL) {
        return create_sender((cairo_surface_t*)i->surface, name);
    }
    return 0;
}

static int ndi_mod_destroy_image_sender(lua_State *l) {
    lua_check_num_args(1);
    _image_t *i = _image_check(l, 1);

    if (i->surface != NULL) {
        return destroy_sender((cairo_surface_t*)i->surface);
    }
    return 0;
}

static int ndi_mod_version(lua_State *l) {
    lua_check_num_args(0);
    lua_pushstring(l, NDI_MOD_VERSION);
    return 1;
}

//
// module definition
//

static const luaL_Reg mod[] = {
    {NULL, NULL}
};

static luaL_Reg func[] = {
    {"init", ndi_mod_init},
    {"cleanup", ndi_mod_cleanup},
    {"init_audio", ndi_mod_init_audio},
    {"cleanup_audio", ndi_mod_cleanup_audio},
    {"update", ndi_mod_update},
    {"start", ndi_mod_start},
    {"stop", ndi_mod_stop},
    {"is_running", ndi_mod_is_running},
    {"create_image_sender", ndi_mod_create_image_sender},
    {"destroy_image_sender", ndi_mod_destroy_image_sender},
    {"version", ndi_mod_version},
    {NULL, NULL}
};

NDI_MOD_API int luaopen_ndi_mod(lua_State *L) {
    lua_newtable(L);

    for (int i = 0; mod[i].name; i++) {
        mod[i].func(L);
    }

    luaL_setfuncs(L, func, 0);

    lua_pushstring(L, "VERSION");
    lua_pushstring(L, NDI_MOD_VERSION);
    lua_rawset(L, -3);

    return 1;
}
