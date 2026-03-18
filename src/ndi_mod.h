#ifndef __NDI_MOD_H__
#define __NDI_MOD_H__

#define NDI_MOD_VERSION "0.3"

#ifndef NDI_MOD_API
#define NDI_MOD_API __attribute__ ((visibility ("default")))
#endif

extern "C" {
// cairo
#include <cairo.h>

#include "lua.h"
#include "lauxlib.h"

NDI_MOD_API int luaopen_ndi_mod(lua_State *L);

} // extern "C"

// ndi
#include <Processing.NDI.Lib.h>
#include <cstring>
#include <string>

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
    SenderRecord();
    SenderRecord(const SenderRecord& other) = delete;
    SenderRecord(SenderRecord&& other);
    SenderRecord(cairo_surface_t* surface, const char* name);
    ~SenderRecord();
    void send();
};

#endif
