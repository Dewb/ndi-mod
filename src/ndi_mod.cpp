#include "ndi_mod.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <cstring>

// lua
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

extern "C" {
// matron
#include "event_types.h"
#include "event_custom.h"
#include "events.h"
}

static bool quitting = false;
static bool running = false;

#include <Processing.NDI.Lib.h>

NDIlib_send_instance_t send_instance = NULL;
NDIlib_video_frame_v2_t ndi_norns_frame;


//
// custom event and behavior
//

static int _report(lua_State *lvm, int status) {
    // if pcall was not successful print the error and pop it off the stack to
    // prevent overflow
    if (status != LUA_OK) {
        const char *msg = lua_tostring(lvm, -1);
        lua_writestringerror("%s\n", msg);
        lua_pop(lvm, 1);
    }
    return status;
}

static void ndi_mod_weave_op(lua_State *lvm, void *value, void *context) {
    uint32_t *cp = static_cast<uint32_t*>(value);
    std::cout << "weave_op: called, counter = " << *cp << std::endl;

    // call a global lua function with current counter value
    lua_getglobal(lvm, "ndi_mod_handler");
    lua_pushinteger(lvm, *cp);
    _report(lvm, lua_pcall(lvm, 1, 0, 0)); // one argument, zero results, default error message
}

static void ndi_mod_free_op(void *value, void *context) {
    // nothing to do here since value is just a pointer to the global counter
    uint32_t *cp = static_cast<uint32_t*>(value);
    std::cout << "free_op: called" << std::endl;
}

static struct event_custom_ops ndi_mod_ops = {
    .type_name = "ndi_mod",
    .weave = &ndi_mod_weave_op,
    .free = &ndi_mod_free_op,
};


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

        running = false;
    }

    return 0;
}

static int ndi_mod_update(lua_State *L) {
    if (running) {
        uint8_t img[] = {
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

LUA_EVENT_DEMO_API int luaopen_ndi_mod(lua_State *L) {
    lua_newtable(L);

    for (int i = 0; mod[i].name; i++) {
        mod[i].func(L);
    }

    luaL_setfuncs(L, func, 0);

    lua_pushstring(L, "VERSION");
    lua_pushstring(L, LUA_EVENT_DEMO_VERSION);
    lua_rawset(L, -3);

    return 1;
}