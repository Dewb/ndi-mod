#ifndef __NDI_MOD_H__
#define __NDI_MOD_H__

#define NDI_MOD_VERSION "0.1"

#ifndef NDI_MOD_API
#define NDI_MOD_API __attribute__ ((visibility ("default")))
#endif

extern "C" {

#include "lua.h"
#include "lauxlib.h"

NDI_MOD_API int luaopen_ndi_mod(lua_State *L);

} // extern "C"

#endif