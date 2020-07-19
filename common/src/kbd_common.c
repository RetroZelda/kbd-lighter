#include "kbd_common.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

void read_led_state(int handle, KeyboardLEDState* kbd)
{
    int data_read = read(handle, &kbd->m_Color.raw, 6);
    if(data_read < 0)
    {
        printf("Read %d bytes with error 0x%X(%d) - %s\n", data_read, errno, errno, strerror(errno));
    }
    assert(data_read <= 6);
}

int read_led_brightness(int handle)
{
    static char buffer[4];
    int data_read = pread(handle, buffer, 4, 0);
    assert(data_read > 0 && data_read <= 4);
    buffer[3] = (char)0;

    return atoi(buffer);
}

int read_bit_as_bool(int handle)
{
    static char buffer[2];
    int data_read = pread(handle, buffer, 2, 0);
    assert(data_read <= 2 && data_read >= 0);
    return buffer[0] == '1' ? 1 : 0;
}

void write_led_state(int handle, KeyboardLEDState* kbd)
{
    static char buffer[7];
    snprintf(buffer, 7, "%.7s", kbd->m_Color.raw); // 7 because null terminator
    int data_written = write(handle, &buffer, 6); // 6 to ignore the null terminator
    if(data_written < 0)
    {
        printf("Wrote %d bytes with error 0x%X(%d) - %s\n", data_written, errno, errno, strerror(errno));
    }
}

void write_led_brightness(int handle, KeyboardLEDState* kbd)
{
    static char buffer[4];
    snprintf(buffer, 4, "%d", kbd->m_Brightness.value);
    int data_writted = write(handle, &buffer, 4);
    assert(data_writted == 4);
}

void print_led_state(KeyboardLEDState* kbd)
{
    printf("LEDS: %.6s (%X, %X, %X)\n", kbd->m_Color.raw
                                      , kbd->m_Color.values.R
                                      , kbd->m_Color.values.G
                                      , kbd->m_Color.values.B);
    printf("Brightness: %d\n", kbd->m_Brightness.value);
    printf("Max Brightness: %d\n", kbd->m_MaxBrightness.value);
}