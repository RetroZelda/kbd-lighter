
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "kbd_common.h"
#include "kbd_breathe.h"
#include "kbd_screen.h"
#include "kbd_audio.h"
#include "kbd_lock.h"

typedef struct _run_mode_funcs
{
    void (*run)(KeyboardLEDState *);
    void (*setup)(KeyboardLEDState *, void *);
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
static RunMode s_led_mode = RM_SCREEN;
static RunMode s_brightness_mode = RM_BREATHE;
static KeyboardLEDState s_led_state;
static KeyboardLEDState s_brightness_state;
static KeyboardLEDState s_final_state;
static bool s_run;

void merge_keyboard_states()
{
    memcpy(&s_final_state.m_Color, &s_led_state.m_Color, sizeof(RGB));
    memcpy(&s_final_state.m_Brightness, &s_brightness_state.m_Brightness, sizeof(Brightness));
}

void lighterd_setup(void **configurations_array)
{
    // read the default LED state
    read_led_state(s_final_state.m_ColorFile.m_Handle, &s_final_state);
    s_final_state.m_Brightness.value = (unsigned char)read_led_brightness(s_final_state.m_BrightnessFile.m_Handle);
    s_final_state.m_MaxBrightness.value = (unsigned char)read_led_brightness(s_final_state.m_MaxBrightnessFile.m_Handle);

    // place the defaults into the separations
    memcpy(&s_led_state, &s_final_state, sizeof(KeyboardLEDState));
    memcpy(&s_brightness_state, &s_final_state, sizeof(KeyboardLEDState));

    // run the startup of the desired mode(s)
    if (s_led_mode == s_brightness_mode)
    {
        s_run_modes[s_led_mode].setup(&s_final_state, configurations_array[s_led_mode]);
    }
    else
    {
        s_run_modes[s_led_mode].setup(&s_led_state, configurations_array[s_led_mode]);
        s_run_modes[s_brightness_mode].setup(&s_brightness_state, configurations_array[s_brightness_mode]);
        merge_keyboard_states();
    }
}

void lighterd_run()
{
    if (s_led_mode == s_brightness_mode)
    {
        s_run_modes[s_led_mode].run(&s_final_state);
    }
    else
    {
        s_run_modes[s_led_mode].run(&s_led_state);
        s_run_modes[s_brightness_mode].run(&s_brightness_state);
        merge_keyboard_states();
    }
}

void lighterd_shutdown()
{
    if (s_led_mode == s_brightness_mode)
    {
        s_run_modes[s_led_mode].shutdown();
    }
    else
    {
        s_run_modes[s_led_mode].shutdown();
        s_run_modes[s_brightness_mode].shutdown();
    }
}

void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
    s_run = false;
    lighterd_shutdown();
}

void catch_sigterm()
{
    memset(&s_sigaction, 0, sizeof(s_sigaction));
    s_sigaction.sa_sigaction = sig_term_handler;
    s_sigaction.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &s_sigaction, NULL);
}

int main(int argc, char **argv)
{
    catch_sigterm();
    const char *max_brightnes_location = "/sys/class/leds/system76_acpi::kbd_backlight/max_brightness";
    const char *brightnes_location = "/sys/class/leds/system76_acpi::kbd_backlight/brightness";
    const char *led_location = "/sys/class/leds/system76_acpi::kbd_backlight/color";

    // open each of the desired locations with LED data
    s_final_state.m_ColorFile.m_Handle = open(led_location, O_RDONLY, 0);
    if (s_final_state.m_ColorFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", led_location, errno, errno);
        return errno;
    }

    s_final_state.m_BrightnessFile.m_Handle = open(brightnes_location, O_RDONLY, 0);
    if (s_final_state.m_BrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", brightnes_location, errno, errno);
        return errno;
    }

    s_final_state.m_MaxBrightnessFile.m_Handle = open(max_brightnes_location, O_RDONLY, 0);
    if (s_final_state.m_MaxBrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" with errno: 0x%X(%d)\n", max_brightnes_location, errno, errno);
        return errno;
    }

    // setup the default configurations
    BreatheConfig breathe_config = {0, 10000};
    ScreenConfig screen_config = {0.9, 1, 1, 10000, 10000};
    AudioConfig audio_config = {0.01f, 0.5f, 1.0f, 2000};
    void *configurations[RM_COUNT] =
        {
            &breathe_config,
            &screen_config,
            &audio_config,
            NULL};

    // TODO: prep to make process daemon

    // open the unix socket to write our obtained led/brightness
    struct sockaddr_un server_addr;
    int server_handle;

    if ((server_handle = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(-1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_ADDR, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_ADDR);
    size_t addr_size = sizeof(server_addr.sun_family) + strlen(server_addr.sun_path);
    if (bind(server_handle, (struct sockaddr *)&server_addr, addr_size) == -1)
    {
        perror("bind error");
        exit(-1);
    }

    if (listen(server_handle, 1) == -1)
    {
        perror("listen error");
        exit(1);
    }

    // setup and run our actual stuff

    int client_handle = -1;
    s_run = true;
    while (s_run)
    {
        if(client_handle == -1)
        {
            client_handle = accept(server_handle, NULL, NULL);
            if (client_handle == -1)
            {
                perror("accept error");
                continue;
            }

            perror("accept");
            int flags = fcntl(client_handle, F_GETFL, 0);
            if(flags == -1)
            {
                perror("fcntl get error");
                exit(-1);
            }

            if(fcntl(client_handle, F_SETFL, flags | O_NONBLOCK) == -1)
            {
                perror("fcntl set error");
                exit(-1);
            }


            lighterd_setup(configurations);
        }
        else
        {
            lighterd_run();

            // write our final state data out
            ssize_t write_size = write(client_handle, &s_final_state, sizeof(LEDState));
            if (write_size != sizeof(LEDState))
            {
                if (write_size > 0)
                {
                    printf("partial write");
                }
                else
                {
                    switch(errno)
                    {
                        case EWOULDBLOCK:
                        break;
                        default:
                            perror("write error");
                            close(client_handle);
                            client_handle = -1;

                            lighterd_shutdown();
                            break;
                    }
                }
            }
            //print_led_state(&s_final_state);
        }
        usleep(1);
    }

    close(s_final_state.m_ColorFile.m_Handle);
    close(s_final_state.m_BrightnessFile.m_Handle);
    close(s_final_state.m_MaxBrightnessFile.m_Handle);

    return 0;
}