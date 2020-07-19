#include "kbd_lock.h"

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

static LockConfig s_config = {320000};

KeyboardLockState s_cur_lock_state;
KeyboardLockState s_prev_lock_state;
static int s_scrolllock_handle;
static int s_capslock_handle;
static int s_numlock_handle;

void lock_run(KeyboardLEDState* led_state)
{
    // read the current lock state
    int num_lock = read_bit_as_bool(s_numlock_handle);
    int caps_lock = read_bit_as_bool(s_capslock_handle);
    int scroll_lock = read_bit_as_bool(s_scrolllock_handle);
    s_cur_lock_state.m_Values = 0;
    s_cur_lock_state.m_Values |= (num_lock != 0) ? 0 : KBD_LOCK_NUM;
    s_cur_lock_state.m_Values |= (caps_lock != 0) ? 0 : KBD_LOCK_CAPS;
    s_cur_lock_state.m_Values |= (scroll_lock != 0) ? 0 : KBD_LOCK_SCROLL;
    //printf("Cur Lock State: %d(%d %d %d)\n", s_cur_lock_state.m_Values, scroll_lock, caps_lock, num_lock);
    //printf("PrevLock State: %d(%d %d %d)\n", s_prev_lock_state.m_Values, scroll_lock, caps_lock, num_lock);
    if(s_cur_lock_state.m_Values != s_prev_lock_state.m_Values)
    {
        s_prev_lock_state = s_cur_lock_state;
        
        // set the leds to reflect this state
        led_state->m_Color.values.R = ((s_cur_lock_state.m_Values & KBD_LOCK_SCROLL) == 0) ? COLOR_MIN : COLOR_MAX;
        led_state->m_Color.values.G = ((s_cur_lock_state.m_Values & KBD_LOCK_CAPS) == 0) ? COLOR_MIN : COLOR_MAX;
        led_state->m_Color.values.B = ((s_cur_lock_state.m_Values & KBD_LOCK_NUM) == 0) ? COLOR_MIN : COLOR_MAX;
    }

    usleep(s_config.m_SleepTimeMS);
}

void lock_setup(KeyboardLEDState* led_state, LockConfig* config)
{
    if(config)
        s_config = *config;
    const char* scrolllock_location     = "/sys/class/leds/input0::scrolllock/brightness";
    const char* capslock_location       = "/sys/class/leds/input0::capslock/brightness";
    const char* numlock_location        = "/sys/class/leds/input0::numlock/brightness";

    s_scrolllock_handle = open(scrolllock_location, O_RDONLY, 0);
    if(s_scrolllock_handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", scrolllock_location, errno, errno);
    }

    s_capslock_handle = open(capslock_location, O_RDONLY, 0);
    if(s_capslock_handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", capslock_location, errno, errno);
    }

    s_numlock_handle = open(numlock_location, O_RDONLY, 0);
    if(s_numlock_handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", numlock_location, errno, errno);
    }

    s_cur_lock_state.m_Values = 0;
    s_prev_lock_state.m_Values = 0;
}

void lock_shutdown()
{
    close(s_scrolllock_handle);
    close(s_capslock_handle);
    close(s_numlock_handle);
}
