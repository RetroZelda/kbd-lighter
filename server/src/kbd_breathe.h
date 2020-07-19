#ifndef KBD_BREATHE
#define KBD_BREATHE

#include "kbd_common.h"

typedef struct _breate_config
{
    unsigned int m_ChangeColor; // 0 or 1
    unsigned int m_SleepTimeMS; // time to sleep between updates in micro seconds
} BreatheConfig;

void breathe_run(KeyboardLEDState* led_state);
void breathe_setup(KeyboardLEDState* led_state, BreatheConfig* config);
void breathe_shutdown();

#endif