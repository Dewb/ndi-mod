#ifndef __NDI_MOD_AUDIO_H__
#define __NDI_MOD_AUDIO_H__

void initialize_jack(const char** output_ports, int num_channels);
void cleanup_jack();

#endif
