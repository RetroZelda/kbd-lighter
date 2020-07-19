#ifndef KBD_SCREEN
#define KBD_SCREEN

#include "kbd_common.h"

typedef struct _screen_config
{
    float m_LerpSmooth;
    unsigned int m_PixelStrideRow;
    unsigned int m_PixelStrideCol;
    unsigned int m_SleepTimeMS; // time to sleep between updates in micro seconds
    unsigned int m_ThreadSleepTimeMS; // time to sleep between updates in micro seconds
} ScreenConfig;

void screen_run(KeyboardLEDState* led_state);
void screen_setup(KeyboardLEDState* led_state, ScreenConfig* config);
void screen_shutdown();

#endif // KBD_SCREEN