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

int main(int argc, char **argv)
{
    const char *max_brightnes_location = "/sys/class/leds/system76_acpi::kbd_backlight/max_brightness";
    const char *brightnes_location = "/sys/class/leds/system76_acpi::kbd_backlight/brightness";
    const char *led_location = "/sys/class/leds/system76_acpi::kbd_backlight/color";
    KeyboardLEDState led_state;

    led_state.m_ColorFile.m_Handle = open(led_location, O_RDWR, 0);
    if (led_state.m_ColorFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" - %s\n", led_location, strerror(errno));
        return errno;
    }

    led_state.m_BrightnessFile.m_Handle = open(brightnes_location, O_RDWR, 0);
    if (led_state.m_BrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" - %s\n", brightnes_location, strerror(errno));
        return errno;
    }

    led_state.m_MaxBrightnessFile.m_Handle = open(max_brightnes_location, O_RDONLY, 0);
    if (led_state.m_MaxBrightnessFile.m_Handle == -1)
    {
        printf("failed to open \"%s\" - %s\n", max_brightnes_location, strerror(errno));
        return errno;
    }

    // read the default LED state
    read_led_state(led_state.m_ColorFile.m_Handle, &led_state);
    led_state.m_Brightness.value = (unsigned char)read_led_brightness(led_state.m_BrightnessFile.m_Handle);
    led_state.m_MaxBrightness.value = (unsigned char)read_led_brightness(led_state.m_MaxBrightnessFile.m_Handle);

    // open the unix socket to read from our obtained led/brightness
    struct sockaddr_un server_addr;
    int client_handle;

    if ((client_handle = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(-1);
    }

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

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_ADDR, sizeof(server_addr.sun_path) - 1);

    size_t addr_size = sizeof(server_addr.sun_family) + strlen(server_addr.sun_path);
    if (connect(client_handle, (struct sockaddr *)&server_addr, addr_size) == -1)
    {
        perror("connect error");
        exit(-1);
    }

    ssize_t read_data = 0;
    u_int32_t counter = 0;
    do
    {
        static LEDState buffer[512];
        read_data = read(client_handle, &buffer, sizeof(buffer));
        if(read_data > 0)
        {
            // int num_read = read_data / sizeof(LEDState);
            memcpy(&led_state, &buffer[0], sizeof(LEDState));
            // printf("read %d bytes for %d structs after %d blocks\n", (int)read_data, num_read, counter);
            write_led_brightness(led_state.m_BrightnessFile.m_Handle, &led_state);
            write_led_state(led_state.m_ColorFile.m_Handle, &led_state);
            // print_led_state(&led_state);
            counter = 0;
        }
        else
        {
            switch(errno)
            {
                case EWOULDBLOCK:
                    if(++counter > 500)
                    {
                        printf("timed out\n");
					    goto CLOSE;
                    }
                break;
                default:
                    perror("read error");
					goto CLOSE;
                    break;
            }
        }
        
        usleep(8000u);
    } while(true);
CLOSE:
    close(client_handle);
    close(led_state.m_ColorFile.m_Handle);
    close(led_state.m_BrightnessFile.m_Handle);
    close(led_state.m_MaxBrightnessFile.m_Handle);

    return 0;
}
