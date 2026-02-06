#include "ndi_mod_audio.h"

#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include <string>

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
static jack_client_t* jack_client = NULL;
static std::vector<jack_port_t*> jack_ports;
static float* jack_audio_buffer = NULL;
static int num_channels = 0;
static NDIlib_send_instance_t ndi_audio_sender = NULL;
static NDIlib_audio_frame_v2_t ndi_audio_frame;

static int jack_process_callback(jack_nframes_t nframes, void* arg) {
    if (!running || !ndi_audio_sender || nframes > NDI_JACK_MAX_FRAMES)
        return 0;

    // copy each channel into planar buffer position
    for (int ch = 0; ch < num_channels; ch++) {
        float* buf = (float*)jack_port_get_buffer(jack_ports[ch], nframes);
        memcpy(jack_audio_buffer + (ch * nframes), buf, nframes * sizeof(float));
    }

    ndi_audio_frame.no_samples = nframes;
    ndi_audio_frame.channel_stride_in_bytes = nframes * sizeof(float);
    NDIlib_send_send_audio_v2(ndi_audio_sender, &ndi_audio_frame);

    return 0;
}

static void jack_shutdown_callback(void* arg) {
    MSG("JACK server shut down unexpectedly");
    jack_client = NULL;
    jack_ports.clear();
    ndi_audio_sender = NULL;
}

void initialize_jack(const char** output_ports, int channel_count) {
    if (channel_count <= 0) {
        MSG("JACK: invalid channel count, continuing without audio");
        return;
    }

    num_channels = channel_count;

    jack_status_t status;
    jack_client = jack_client_open("ndi-mod", JackNoStartServer, &status);
    if (!jack_client) {
        MSG("JACK: could not open client (server not running?), continuing without audio");
        return;
    }

    // allocate the audio buffer dynamically
    jack_audio_buffer = new float[num_channels * NDI_JACK_MAX_FRAMES];

    // register N input ports
    jack_ports.reserve(num_channels);
    for (int i = 0; i < num_channels; i++) {
        std::string port_name = "input_" + std::to_string(i + 1);
        jack_port_t* port = jack_port_register(jack_client, port_name.c_str(),
            JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        if (!port) {
            MSG("JACK: failed to register port " << port_name << ", continuing without audio");
            // clean up already registered ports
            for (auto& p : jack_ports) {
                jack_port_unregister(jack_client, p);
            }
            jack_ports.clear();
            jack_client_close(jack_client);
            jack_client = NULL;
            delete[] jack_audio_buffer;
            jack_audio_buffer = NULL;
            num_channels = 0;
            return;
        }
        jack_ports.push_back(port);
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
    ndi_audio_frame.no_channels = num_channels;
    ndi_audio_frame.no_samples = 0;
    ndi_audio_frame.p_data = jack_audio_buffer;
    ndi_audio_frame.channel_stride_in_bytes = 0;

    jack_set_process_callback(jack_client, jack_process_callback, NULL);
    jack_on_shutdown(jack_client, jack_shutdown_callback, NULL);

    if (jack_activate(jack_client)) {
        MSG("JACK: failed to activate client, continuing without audio");
        for (auto& p : jack_ports) {
            jack_port_unregister(jack_client, p);
        }
        jack_ports.clear();
        jack_client_close(jack_client);
        jack_client = NULL;
        delete[] jack_audio_buffer;
        jack_audio_buffer = NULL;
        ndi_audio_sender = NULL;
        num_channels = 0;
        return;
    }

    // connect to specified output ports
    for (int i = 0; i < num_channels; i++) {
        std::string our_port = "ndi-mod:input_" + std::to_string(i + 1);
        if (jack_connect(jack_client, output_ports[i], our_port.c_str()) != 0) {
            MSG("JACK: could not connect to " << output_ports[i] << " (connect manually)");
        }
    }

    MSG("JACK audio client initialized (" << num_channels << " channels, sample rate: " << ndi_audio_frame.sample_rate << ")");
}

void cleanup_jack() {
    if (jack_client) {
        jack_deactivate(jack_client);
        jack_client_close(jack_client);
        jack_client = NULL;
        jack_ports.clear();
        ndi_audio_sender = NULL;
        MSG("JACK audio client closed");
    }
    if (jack_audio_buffer) {
        delete[] jack_audio_buffer;
        jack_audio_buffer = NULL;
    }
    num_channels = 0;
}
