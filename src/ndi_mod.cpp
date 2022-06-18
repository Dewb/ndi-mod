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

static bool quitting = false;
static bool running = false;

#include <Processing.NDI.Lib.h>

NDIlib_send_instance_t send_instance = NULL;
NDIlib_video_frame_v2_t ndi_norns_frame;


static int ndi_mod_start(lua_State *L) {

    if (!running) {
        quitting = false;

        if (!NDIlib_initialize()) {
            std::cout << "Error initializing NDI library";
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
                std::cout << "Error creating NDI sender";
            }
        }

        ndi_norns_frame.xres = 128;
        ndi_norns_frame.yres = 64;
        ndi_norns_frame.FourCC = NDIlib_FourCC_type_RGBX;
        ndi_norns_frame.frame_format_type = NDIlib_frame_format_type_progressive;
        ndi_norns_frame.line_stride_in_bytes = ndi_norns_frame.xres * 4 * sizeof(uint8_t);
        ndi_norns_frame.p_data = (uint8_t*)malloc(ndi_norns_frame.line_stride_in_bytes * ndi_norns_frame.yres);
        memset(ndi_norns_frame.p_data, 0, ndi_norns_frame.line_stride_in_bytes * ndi_norns_frame.yres);

        std::cout << "NDI server initialized";
        running = true;
    }

    return 0;
}

static int ndi_mod_stop(lua_State *L) {
    
    if (running) {
        quitting = true;
        if (ndi_norns_frame.p_data) {
            free(ndi_norns_frame.p_data);
        }

        NDIlib_send_destroy(send_instance);
        NDIlib_destroy();

        std::cout << "NDI server stopped";
        running = false;
    }

    return 0;
}

static int ndi_mod_update(lua_State *L) {
    if (running) {
        /*uint8_t img[] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
            0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0,
        };
        memcpy(ndi_norns_frame.p_data, img, 64);
        */

        // this is probably not safe
        // current context is not guaranteed to be the primary context
        cairo_t* ctx = (cairo_t*)screen_context_get_current();
        cairo_surface_t* surface = cairo_get_target(ctx);
        cairo_surface_flush(surface);
        unsigned char* data = cairo_image_surface_get_data(surface);
        if (data != NULL) {
            memcpy(ndi_norns_frame.p_data, data, 128*64*4);
        }

        NDIlib_send_send_video_async_v2(send_instance, &ndi_norns_frame);
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
    {"start", ndi_mod_start},
    {"stop", ndi_mod_stop},
    {"update", ndi_mod_update},
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