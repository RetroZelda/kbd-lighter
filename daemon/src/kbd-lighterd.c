#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include "kbd_common.h"
#include "kbd_breathe.h"
#include "kbd_screen.h"
#include "kbd_audio.h"
#include "kbd_lock.h"

typedef struct _run_mode_funcs
{
    void (*run)(KeyboardLEDState*);
    void (*setup)(KeyboardLEDState*, void*);
    void (*shutdown)();
} RunModeFuncs;

typedef enum _run_mode
{
    RM_BREATHE,
    RM_SCREEN,
    RM_AUDIO,
    RM_LOCK,

    RM_COUNT
} RunMode;

static RunModeFuncs s_run_modes[RM_COUNT] =
{
    {&breathe_run, &breathe_setup, &breathe_shutdown},
    {&screen_run, &screen_setup, &screen_shutdown},
    {&audio_run, &audio_setup, &audio_shutdown},
    {&lock_run, &lock_setup, &lock_shutdown},
};

static struct sigaction s_sigaction;
static RunMode s_run_mode = RM_AUDIO;
static bool s_run;

void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
    s_run = false;
    s_run_modes[s_run_mode].shutdown();
}

void catch_sigterm()
{
    memset(&s_sigaction, 0, sizeof(s_sigaction));
    s_sigaction.sa_sigaction = sig_term_handler;
    s_sigaction.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &s_sigaction, NULL);
}

int main(int argc, char** argv)
{
    catch_sigterm();
    const char* max_brightnes_location  = "/sys/class/leds/system76_acpi::kbd_backlight/max_brightness";
    const char* brightnes_location      = "/sys/class/leds/system76_acpi::kbd_backlight/brightness";
    const char* led_location            = "/sys/class/leds/system76_acpi::kbd_backlight/color";
    KeyboardLEDState led_state;

    //led_state.m_ColorFile.m_Handle = open(led_location, O_RDWR, 0);
    //if(led_state.m_ColorFile.m_Handle == -1)
    //{
    //    printf("failed to open \"%s\" with errno: 0x%X(%d)\n", led_location, errno, errno);
    //    return errno;
    //}
//
    //led_state.m_BrightnessFile.m_Handle = open(brightnes_location, O_RDWR, 0);
    //if(led_state.m_BrightnessFile.m_Handle == -1)
    //{
    //    printf("failed to open \"%s\" with errno: 0x%X(%d)\n", brightnes_location, errno, errno);
    //    return errno;
    //}

    led_state.m_MaxBrightnessFile.m_Handle = open(max_brightnes_location, O_RDONLY, 0);
    if(led_state.m_MaxBrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", max_brightnes_location, errno, errno);
        return errno;
    }

    // read the default LED state
    //read_led_state(led_state.m_ColorFile.m_Handle, &led_state);
    //led_state.m_Brightness.value = (unsigned char)read_led_brightness(led_state.m_BrightnessFile.m_Handle);
    led_state.m_MaxBrightness.value = (unsigned char)read_led_brightness(led_state.m_MaxBrightnessFile.m_Handle);
    
    s_run = true;

    BreatheConfig breathe_config = {0, 2000};
    ScreenConfig screen_config = {0.00001f, 1, 1, 100000, 40000};
    AudioConfig audio_config = {2.0f, 2000};
    void* configurations[RM_COUNT] = 
    {
        &breathe_config,
        &screen_config,
        &audio_config,
        NULL
    };

    s_run_modes[s_run_mode].setup(&led_state, configurations[s_run_mode]);
    while(s_run)
    {
        s_run_modes[s_run_mode].run(&led_state);

        // write_led_brightness(led_state.m_BrightnessFile.m_Handle, &led_state);
        // write_led_state(led_state.m_ColorFile.m_Handle, &led_state);
        print_led_state(&led_state);
    }
    s_run_modes[s_run_mode].shutdown();

    printf("\n");

    close(led_state.m_ColorFile.m_Handle);
    close(led_state.m_BrightnessFile.m_Handle);
    close(led_state.m_MaxBrightnessFile.m_Handle);

    return 0;
}