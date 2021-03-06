#include "kbd_breathe.h"

#include <unistd.h>
#include <stdio.h>
#include <time.h>

static int s_brightness_adjustment;
static int s_color_counter;
static BreatheConfig s_config = {1, 2000};

static double s_prev_time = 0.0f;

void breathe_run(KeyboardLEDState* led_state)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    double cur_time = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
    double time_dif = cur_time - s_prev_time;
    double sleep_time = (double)s_config.m_SleepTimeMS / 1000000.0;

    if(time_dif >= sleep_time)
    {
        s_prev_time = cur_time;
        led_state->m_Brightness.value += s_brightness_adjustment;
        if(led_state->m_Brightness.value >= led_state->m_MaxBrightness.value ||
            led_state->m_Brightness.value <= 1)
        {
            s_brightness_adjustment *= -1;

            if(led_state->m_Brightness.value <= 1)
            {
                s_color_counter = (s_color_counter %7) + 1; // 1-7 range
                
                if(s_config.m_ChangeColor == 1)
                {
                    led_state->m_Color.values.R = ((s_color_counter & (1 << 0)) == 0) ? COLOR_MIN : COLOR_MAX;
                    led_state->m_Color.values.G = ((s_color_counter & (1 << 1)) == 0) ? COLOR_MIN : COLOR_MAX;
                    led_state->m_Color.values.B = ((s_color_counter & (1 << 2)) == 0) ? COLOR_MIN : COLOR_MAX;
                }
            }
        }
    }
}

void breathe_setup(KeyboardLEDState* led_state, BreatheConfig* config)
{
    if(config)
        s_config = *config;
    s_brightness_adjustment = led_state->m_Brightness.value > 0 ? -1 : 1;
    s_color_counter =   (led_state->m_Color.values.R == COLOR_MIN ? 0 : (1 << 0)) | 
                        (led_state->m_Color.values.G == COLOR_MIN ? 0 : (1 << 1)) |
                        (led_state->m_Color.values.B == COLOR_MIN ? 0 : (1 << 2));
}

void breathe_shutdown()
{

}
