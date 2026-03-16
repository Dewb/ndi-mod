#include "ndi_mod.h"

#include <iostream>
#include <map>
#include <string.h>

// lua
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

extern "C" {
// matron
#include "lua_eval.h"
#include "hardware/screen.h"
// cairo
#include <cairo.h>
// local copies of private weaver types/methods
#include <weaver_image.h>
}

// ndi
#include <Processing.NDI.Lib.h>


#define MSG(contents) \
   std::cerr << "ndi-mod: " << contents << "\n"


class SenderRecord
{
public:
    cairo_surface_t* surface;
    NDIlib_send_instance_t sender;
    std::string sender_name;
    unsigned char* buffer;
    size_t buffer_size;
    NDIlib_video_frame_v2_t frame;
    int frame_divisor;
    int frame_counter;

    SenderRecord()
    : surface(NULL), sender(NULL), sender_name(""), buffer(NULL), buffer_size(0)
    {}

    SenderRecord(const SenderRecord& other) = delete;
    SenderRecord(SenderRecord&& other) {
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

    SenderRecord(cairo_surface_t* surface, const char* name)
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

    ~SenderRecord() {
        if (sender != NULL) {
            NDIlib_send_destroy(sender);
            free(buffer);
            MSG("NDI sender destroyed");
        }
        if (surface != NULL) {
            cairo_surface_destroy(surface); // decrease refcount
        }
    }

    void send();
};

std::map<cairo_surface_t*, SenderRecord> surface_sender_map;

static bool running = false;
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

//
// module definition
//

static const luaL_Reg mod[] = {
    {NULL, NULL}
};

static luaL_Reg func[] = {
    {"init", ndi_mod_init},
    {"cleanup", ndi_mod_cleanup},
    {"update", ndi_mod_update},
    {"start", ndi_mod_start},
    {"stop", ndi_mod_stop},
    {"is_running", ndi_mod_is_running},
    {"create_image_sender", ndi_mod_create_image_sender},
    {"destroy_image_sender", ndi_mod_destroy_image_sender},
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
