#ifndef KBD_LOCK
#define KBD_LOCK

#include "kbd_common.h"

typedef struct _lock_config
{
    unsigned int m_SleepTimeMS; // time to sleep between updates in micro seconds
} LockConfig;

void lock_run(KeyboardLEDState* led_state);
void lock_setup(KeyboardLEDState* led_state, LockConfig* config);
void lock_shutdown();

#endif // KBD_LOCK