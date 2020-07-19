#include "kbd_breathe.h"

#include <unistd.h>
#include <stdio.h>

static int s_brightness_adjustment;
static int s_color_counter;
static BreatheConfig s_config = {1, 2000};

void breathe_run(KeyboardLEDState* led_state)
{
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
    usleep(s_config.m_SleepTimeMS); // 2ms
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
