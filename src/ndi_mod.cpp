#include "ndi_mod.h"

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
// cairo
#include <cairo.h>
// local copies of private weaver types/methods
#include <weaver_image.h>
}

// ndi
#include <Processing.NDI.Lib.h>

std::map<cairo_surface_t*, NDIlib_send_instance_t> surface_sender_map;
NDIlib_video_frame_v2_t ndi_norns_frame;

static bool running = false;
static bool initialized = false;
static bool failed = false;

//
// core functions
//

int create_sender(cairo_surface_t* surface, const char* name) {
    NDIlib_send_create_t send_create;
    send_create.p_ndi_name = name;
    send_create.p_groups = NULL;
    send_create.clock_video = false;
    send_create.clock_audio = false;

    NDIlib_send_instance_t send_instance = NDIlib_send_create(&send_create);
    if (!send_instance) {
        std::cerr << "Error creating NDI sender \"" << name << "\"\n";
        return 0;
    }

    surface_sender_map[surface] = send_instance;
    std::cerr << "NDI sender \"" << name << "\" created\n";

    return 0;
}

int destroy_sender(cairo_surface_t* surface) {
    auto it = surface_sender_map.find(surface);
    if (it == surface_sender_map.end()) {
        // no NDI sender registered for this surface
        return 0;
    }

    NDIlib_send_destroy(it->second);
    surface_sender_map.erase(it);
    std::cerr << "NDI sender destroyed\n";
    return 0;
}

int initialize_ndi() {
    if (!initialized && !failed) {
        if (!NDIlib_initialize()) {
            std::cerr << "Error initializing NDI library\n";
            failed = true;
            return 0;
        }

        std::cerr << "NDI service initialized\n";
        initialized = true;

        // NDI video format: 30fps RGBA progressive (with alpha ignored.)
        // norns cairo surfaces are CAIRO_FORMAT_ARGB32 (premultiplied ARGB.)
        // But all four bytes are always the same, so the RGBA/ARGB mismatch
        // doesn't matter, and we can use the surface data directly.
        ndi_norns_frame.frame_rate_N = 30000;
        ndi_norns_frame.frame_rate_D = 1000;
        ndi_norns_frame.FourCC = NDIlib_FourCC_type_RGBX;
        ndi_norns_frame.frame_format_type = NDIlib_frame_format_type_progressive;

        // create the default sender
        cairo_t* ctx = (cairo_t*)screen_context_get_current();
        if (ctx == NULL) {
            return 0;
        }

        cairo_surface_t* surface = cairo_get_target(ctx);
        create_sender(surface, "norns screen");
    }
    return 0;
}

int cleanup_ndi() {
    running = false;
    if (initialized) {
        initialized = false;

        for (auto& kv : surface_sender_map) {
            NDIlib_send_destroy(kv.second);
        }
        surface_sender_map.clear();

        NDIlib_destroy();
        std::cerr << "NDI service stopped\n";
    }
    return 0;
}

int send_surface_as_frame(cairo_surface_t* surface)
{
    if (initialized && !failed) {
        // locate the sender
        auto it = surface_sender_map.find(surface);
        if (it == surface_sender_map.end()) {
            // no NDI sender registered for this surface
            return 0;
        }
        auto send_instance = it->second;

        // prepare the surface
        if (cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE ||
            cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
            return 0;
        }
        cairo_surface_flush(surface);

        // prepare the frame and send it
        unsigned char* data = cairo_image_surface_get_data(surface);
        if (data != NULL) {
            ndi_norns_frame.xres = cairo_image_surface_get_width(surface);
            ndi_norns_frame.yres = cairo_image_surface_get_height(surface);
            ndi_norns_frame.line_stride_in_bytes = cairo_image_surface_get_stride(surface);
            ndi_norns_frame.p_data = data;
            NDIlib_send_send_video_async_v2(send_instance, &ndi_norns_frame);
        }
    }
    return 0;
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
    if (running) {
        cairo_t* ctx = (cairo_t*)screen_context_get_current();
        if (ctx == NULL) {
            return 0;
        }
        cairo_surface_t* surface = cairo_get_target(ctx);
        return send_surface_as_frame(surface);
    }
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