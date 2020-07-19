#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "kbd_common.h"


int main(int argc, char** argv)
{
    const char* max_brightnes_location  = "/sys/class/leds/system76_acpi::kbd_backlight/max_brightness";
    const char* brightnes_location      = "/sys/class/leds/system76_acpi::kbd_backlight/brightness";
    const char* led_location            = "/sys/class/leds/system76_acpi::kbd_backlight/color";
    KeyboardLEDState led_state;

    led_state.m_ColorFile.m_Handle = open(led_location, O_RDWR, 0);
    if(led_state.m_ColorFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" - %s\n", led_location, strerror(errno));
        return errno;
    }

    led_state.m_BrightnessFile.m_Handle = open(brightnes_location, O_RDWR, 0);
    if(led_state.m_BrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" - %s\n", brightnes_location, strerror(errno));
        return errno;
    }

    led_state.m_MaxBrightnessFile.m_Handle = open(max_brightnes_location, O_RDONLY, 0);
    if(led_state.m_MaxBrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" - %s\n", max_brightnes_location, strerror(errno));
        return errno;
    }

    // read the default LED state
    read_led_state(led_state.m_ColorFile.m_Handle, &led_state);
    led_state.m_Brightness.value = (unsigned char)read_led_brightness(led_state.m_BrightnessFile.m_Handle);
    led_state.m_MaxBrightness.value = (unsigned char)read_led_brightness(led_state.m_MaxBrightnessFile.m_Handle);
    
    // write_led_brightness(led_state.m_BrightnessFile.m_Handle, &led_state);
    // write_led_state(led_state.m_ColorFile.m_Handle, &led_state);
    print_led_state(&led_state);

    printf("\n");

    close(led_state.m_ColorFile.m_Handle);
    close(led_state.m_BrightnessFile.m_Handle);
    close(led_state.m_MaxBrightnessFile.m_Handle);

    return 0;
}