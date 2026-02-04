#include "ndi_mod.h"

#include <iostream>
#include <map>
#include <cstring>

#include <jack/jack.h>

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

// jack audio
static const int NDI_JACK_MAX_FRAMES = 8192;
static const int NDI_JACK_NUM_CHANNELS = 2;
static jack_client_t* jack_client = NULL;
static jack_port_t* jack_port_left = NULL;
static jack_port_t* jack_port_right = NULL;
static float jack_audio_buffer[NDI_JACK_NUM_CHANNELS * NDI_JACK_MAX_FRAMES];
static NDIlib_send_instance_t ndi_audio_sender = NULL;
static NDIlib_audio_frame_v2_t ndi_audio_frame;

//
// core functions
//

#define MSG(contents) \
   std::cerr << "ndi-mod: " << contents << "\n"

int create_sender(cairo_surface_t* surface, const char* name) {
    auto it = surface_sender_map.find(surface);
    if (it != surface_sender_map.end()) {
        MSG("an NDI sender already exists for this surface");
        return 0;
    }

    NDIlib_send_create_t send_create;
    send_create.p_ndi_name = name;
    send_create.p_groups = NULL;
    send_create.clock_video = false;
    send_create.clock_audio = false;

    NDIlib_send_instance_t send_instance = NDIlib_send_create(&send_create);
    if (!send_instance) {
        MSG("error creating NDI sender");
        return 0;
    }

    surface_sender_map[surface] = send_instance;
    cairo_surface_reference(surface); // increase refcount
    MSG("NDI sender \"" << name << "\" created");

    return 0;
}

int destroy_sender(cairo_surface_t* surface) {
    auto it = surface_sender_map.find(surface);
    if (it == surface_sender_map.end()) {
        MSG("No NDI sender exists for this surface");
        return 0;
    }

    NDIlib_send_destroy(it->second);
    surface_sender_map.erase(it);
    cairo_surface_destroy(surface); // decrease refcount
    MSG("NDI sender destroyed");
    return 0;
}

//
// jack audio functions
//

static int jack_process_callback(jack_nframes_t nframes, void* arg) {
    if (!running || !ndi_audio_sender || nframes > NDI_JACK_MAX_FRAMES)
        return 0;

    float* left = (float*)jack_port_get_buffer(jack_port_left, nframes);
    float* right = (float*)jack_port_get_buffer(jack_port_right, nframes);

    // copy into planar buffer: channel 0 then channel 1
    memcpy(jack_audio_buffer, left, nframes * sizeof(float));
    memcpy(jack_audio_buffer + nframes, right, nframes * sizeof(float));

    ndi_audio_frame.no_samples = nframes;
    ndi_audio_frame.channel_stride_in_bytes = nframes * sizeof(float);
    NDIlib_send_send_audio_v2(ndi_audio_sender, &ndi_audio_frame);

    return 0;
}

static void jack_shutdown_callback(void* arg) {
    MSG("JACK server shut down unexpectedly");
    jack_client = NULL;
    jack_port_left = NULL;
    jack_port_right = NULL;
    ndi_audio_sender = NULL;
}

static void initialize_jack() {
    jack_status_t status;
    jack_client = jack_client_open("ndi-mod", JackNoStartServer, &status);
    if (!jack_client) {
        MSG("JACK: could not open client (server not running?), continuing without audio");
        return;
    }

    jack_port_left = jack_port_register(jack_client, "input_1",
        JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    jack_port_right = jack_port_register(jack_client, "input_2",
        JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

    if (!jack_port_left || !jack_port_right) {
        MSG("JACK: failed to register ports, continuing without audio");
        jack_client_close(jack_client);
        jack_client = NULL;
        jack_port_left = NULL;
        jack_port_right = NULL;
        return;
    }

    // cache the "screen" sender for audio
    cairo_t* ctx = (cairo_t*)screen_context_get_primary();
    if (ctx) {
        cairo_surface_t* surface = cairo_get_target(ctx);
        auto it = surface_sender_map.find(surface);
        if (it != surface_sender_map.end()) {
            ndi_audio_sender = it->second;
        }
    }

    // pre-fill audio frame descriptor
    memset(&ndi_audio_frame, 0, sizeof(ndi_audio_frame));
    ndi_audio_frame.timecode = NDIlib_send_timecode_synthesize;
    ndi_audio_frame.sample_rate = jack_get_sample_rate(jack_client);
    ndi_audio_frame.no_channels = NDI_JACK_NUM_CHANNELS;
    ndi_audio_frame.no_samples = 0;
    ndi_audio_frame.p_data = jack_audio_buffer;
    ndi_audio_frame.channel_stride_in_bytes = 0;

    jack_set_process_callback(jack_client, jack_process_callback, NULL);
    jack_on_shutdown(jack_client, jack_shutdown_callback, NULL);

    if (jack_activate(jack_client)) {
        MSG("JACK: failed to activate client, continuing without audio");
        jack_client_close(jack_client);
        jack_client = NULL;
        jack_port_left = NULL;
        jack_port_right = NULL;
        ndi_audio_sender = NULL;
        return;
    }

    // auto-connect to crone outputs
    if (jack_connect(jack_client, "crone:output_1", "ndi-mod:input_1") != 0) {
        MSG("JACK: could not connect to crone:output_1 (connect manually)");
    }
    if (jack_connect(jack_client, "crone:output_2", "ndi-mod:input_2") != 0) {
        MSG("JACK: could not connect to crone:output_2 (connect manually)");
    }

    MSG("JACK audio client initialized (sample rate: " << ndi_audio_frame.sample_rate << ")");
}

static void cleanup_jack() {
    if (jack_client) {
        jack_deactivate(jack_client);
        jack_client_close(jack_client);
        jack_client = NULL;
        jack_port_left = NULL;
        jack_port_right = NULL;
        ndi_audio_sender = NULL;
        MSG("JACK audio client closed");
    }
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

        // NDI video format: 60fps RGBA progressive (with alpha ignored.)
        // norns cairo surfaces are CAIRO_FORMAT_ARGB32 (premultiplied ARGB.)
        // But all four bytes are always the same, so the RGBA/ARGB mismatch
        // doesn't matter, and we can use the surface data directly.
        ndi_norns_frame.frame_rate_N = 60000;
        ndi_norns_frame.frame_rate_D = 1000;
        ndi_norns_frame.FourCC = NDIlib_FourCC_type_RGBX;
        ndi_norns_frame.frame_format_type = NDIlib_frame_format_type_progressive;

        // create the default sender
        cairo_t* ctx = (cairo_t*)screen_context_get_primary();
        if (ctx == NULL) {
            return 0;
        }

        cairo_surface_t* surface = cairo_get_target(ctx);
        create_sender(surface, "screen");

        initialize_jack();
    }
    return 0;
}

int cleanup_ndi() {
    running = false;
    cleanup_jack();
    if (initialized) {
        initialized = false;

        for (auto& kv : surface_sender_map) {
            NDIlib_send_destroy(kv.second);
        }
        surface_sender_map.clear();

        NDIlib_destroy();
        MSG("NDI service stopped");
    }
    return 0;
}

static void send_surface_as_frame(cairo_surface_t* surface, NDIlib_send_instance_t send_instance)
{
    if (initialized && !failed) {

        if (cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE ||
            cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
            return;
        }

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
}

static void update_all_surfaces()
{
    if (running)
    {
        for (const auto& entry : surface_sender_map) {
            send_surface_as_frame(entry.first, entry.second);
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
