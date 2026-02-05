#include "ndi_mod_audio.h"

#include <iostream>
#include <map>
#include <cstring>

#include <jack/jack.h>

extern "C" {
#include "hardware/screen.h"
#include <cairo.h>
}

#include <Processing.NDI.Lib.h>

#define MSG(contents) \
   std::cerr << "ndi-mod_audio: " << contents << "\n"

// from ndi_mod.cpp
extern std::map<cairo_surface_t*, NDIlib_send_instance_t> surface_sender_map;
extern bool running;

// jack audio
static const int NDI_JACK_MAX_FRAMES = 8192;
static const int NDI_JACK_NUM_CHANNELS = 2;
static jack_client_t* jack_client = NULL;
static jack_port_t* jack_port_left = NULL;
static jack_port_t* jack_port_right = NULL;
static float jack_audio_buffer[NDI_JACK_NUM_CHANNELS * NDI_JACK_MAX_FRAMES];
static NDIlib_send_instance_t ndi_audio_sender = NULL;
static NDIlib_audio_frame_v2_t ndi_audio_frame;

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

void initialize_jack(const char* output_left, const char* output_right) {
    // default to crone outputs if not specified
    if (!output_left || output_left[0] == '\0') {
        output_left = "crone:output_1";
    }
    if (!output_right || output_right[0] == '\0') {
        output_right = "crone:output_2";
    }

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

    // connect to specified outputs
    if (jack_connect(jack_client, output_left, "ndi-mod:input_1") != 0) {
        MSG("JACK: could not connect to " << output_left << " (connect manually)");
    }
    if (jack_connect(jack_client, output_right, "ndi-mod:input_2") != 0) {
        MSG("JACK: could not connect to " << output_right << " (connect manually)");
    }

    MSG("JACK audio client initialized (sample rate: " << ndi_audio_frame.sample_rate << ")");
}

void cleanup_jack() {
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
