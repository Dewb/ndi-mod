#include "ndi_mod.h"

#include <chrono>
#include <iostream>
#include <cstring>

// lua
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

extern "C" {
// matron
#include "hardware/screen.h"
#include <cairo.h>
}

#include <Processing.NDI.Lib.h>

NDIlib_send_instance_t send_instance = NULL;
NDIlib_video_frame_v2_t ndi_norns_frame;

static bool running = false;
static bool initialized = false;
static bool failed = false;

static int ndi_mod_init(lua_State *L) {

    if (!initialized && !failed) {

        if (!NDIlib_initialize()) {
            std::cerr << "Error initializing NDI library";
            failed = true;
            return 0;
        }

        NDIlib_send_create_t send_create;
        send_create.p_ndi_name = "norns screen";
        send_create.p_groups = NULL;
        send_create.clock_video = false;
        send_create.clock_audio = false;

        if (!send_instance) {
            send_instance = NDIlib_send_create(&send_create);
            if (!send_instance) {
                failed = true;
                std::cerr << "Error creating NDI server";
            }
        }

        ndi_norns_frame.xres = 128;
        ndi_norns_frame.yres = 64;
        ndi_norns_frame.FourCC = NDIlib_FourCC_type_RGBX;
        ndi_norns_frame.frame_format_type = NDIlib_frame_format_type_progressive;
        ndi_norns_frame.line_stride_in_bytes = ndi_norns_frame.xres * 4 * sizeof(uint8_t);
        ndi_norns_frame.p_data = (uint8_t*)malloc(ndi_norns_frame.line_stride_in_bytes * ndi_norns_frame.yres);
        memset(ndi_norns_frame.p_data, 0, ndi_norns_frame.line_stride_in_bytes * ndi_norns_frame.yres);

        std::cerr << "NDI server initialized";
        initialized = true;
    }

    return 0;
}

static int ndi_mod_cleanup(lua_State *L) {

    running = false;

    if (initialized) {
        initialized = false;

        if (ndi_norns_frame.p_data) {
            free(ndi_norns_frame.p_data);
        }

        NDIlib_send_destroy(send_instance);
        NDIlib_destroy();

        std::cerr << "NDI server stopped";
    }

    return 0;
}

static int ndi_mod_send_frame(lua_State *L) {

    ndi_mod_init(L);

    if (initialized && !failed) {

        // this is questionable -- is current context guaranteed to be the primary context?
        cairo_t* ctx = (cairo_t*)screen_context_get_current();
        if (ctx == NULL) {
            return 0;
        }

        cairo_surface_t* surface = cairo_get_target(ctx);
        if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
            return 0;
        }

        cairo_surface_flush(surface);
        unsigned char* data = cairo_image_surface_get_data(surface);
        if (data != NULL) {
            memcpy(ndi_norns_frame.p_data, data, 128*64*4);
        }

        NDIlib_send_send_video_async_v2(send_instance, &ndi_norns_frame);
    }
    return 0;
}

static int ndi_mod_update(lua_State *L) {
    if (running) {
        return ndi_mod_send_frame(L);
    }
    return 0;
}

static int ndi_mod_start(lua_State *L) {
    running = true;
    return 0;
}

static int ndi_mod_stop(lua_State *L) {
    running = false;
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
    {"send_frame", ndi_mod_send_frame},
    {"update", ndi_mod_update},
    {"start", ndi_mod_start},
    {"stop", ndi_mod_stop},
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