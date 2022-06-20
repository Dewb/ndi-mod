// private definitions from weaver.c

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

extern "C" {
#include "hardware/screen.h"
#include <cairo.h>
}

typedef struct {
    screen_surface_t *surface;
    screen_context_t *context;
    const screen_context_t *previous_context;
    char *name;
} _image_t;

static const char *_image_class_name = "norns.image";

_image_t *_image_check(lua_State *l, int arg) {
    void *ud = luaL_checkudata(l, arg, _image_class_name);
    luaL_argcheck(l, ud != NULL, arg, "image object expected");
    return (_image_t *)ud;
}