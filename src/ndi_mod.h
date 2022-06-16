#ifndef __LUA_EVENT_DEMO_H__
#define __LUA_EVENT_DEMO_H__

#define LUA_EVENT_DEMO_VERSION "0.1"

#ifndef LUA_EVENT_DEMO_API
#define LUA_EVENT_DEMO_API __attribute__ ((visibility ("default")))
#endif

extern "C" {

#include "lua.h"
#include "lauxlib.h"

LUA_EVENT_DEMO_API int luaopen_ndi_mod(lua_State *L);

} // extern "C"

#endif