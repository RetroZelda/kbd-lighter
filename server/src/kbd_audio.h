#ifndef KBD_AUDIO
#define KBD_AUDIO

#include "kbd_common.h"

typedef struct _audio_config
{
    float m_Sensitivity;
    unsigned int m_SleepTimeMS; // time to sleep between updates in micro seconds
} AudioConfig;

float audio_get_level();

void audio_run(KeyboardLEDState* led_state);
void audio_setup(KeyboardLEDState* led_state, AudioConfig* config);
void audio_shutdown();

#endif