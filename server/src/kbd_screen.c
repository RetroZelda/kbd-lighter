#include "kbd_screen.h"
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include "utility/color_conversion.h"

static ScreenConfig s_config = {0.5f, 1, 1, 1000, 10000};

static pthread_t s_thread_id;

static pthread_mutex_t s_is_running_lock;
static int s_is_thread_running;

static pthread_mutex_t s_active_rgb_lock;
RGBColor s_active_color_rgb;
RGBColor s_prev_color_rgb;
RGBColor s_cur_color_rgb;

void *thread_run(void *arg)
{
    pthread_detach(pthread_self());

    Display* display = XOpenDisplay((char *)NULL);
    Visual* default_visual = DefaultVisual(display, 0);
    int default_screen = XDefaultScreen(display);
    Screen* display_screen = ScreenOfDisplay(display, 0);
    Window root_window = XRootWindow(display, default_screen);

    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = s_config.m_ThreadSleepTimeMS * 1000;

    int pixel_stride_row = (int)s_config.m_PixelStrideRow;
    int pixel_stride_col = (int)s_config.m_PixelStrideCol;
    if (pixel_stride_row < 0)
        pixel_stride_row *= -1;
    if (pixel_stride_col < 0)
        pixel_stride_col *= -1;

    XShmSegmentInfo segment_info;
    XImage* image = XShmCreateImage(display, default_screen, 24, ZPixmap, NULL, &segment_info, display_screen->width, display_screen->height);
    segment_info.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
    segment_info.shmaddr = image->data = shmat(segment_info.shmid, 0, 0);
    segment_info.readOnly = False;
    XShmAttach(display, &segment_info);

    while (s_is_thread_running)
    {
        XShmGetImage(display, display_screen->root, image, 0, 0, AllPlanes);
        // calculate the average pixel color
        // NOTE: Slow as balls
        unsigned long average_red = 0;
        unsigned long average_green = 0;
        unsigned long average_blue = 0;
        XColor temp_color;
        for (int col = 0; col < display_screen->width; col += pixel_stride_col)
        {
            for (int row = 0; row < display_screen->height; row += pixel_stride_row)
            {
                temp_color.pixel = XGetPixel(image, col, row);
                average_red += (unsigned long)((temp_color.pixel >> 16) & 0xff);
                average_green += (unsigned long)((temp_color.pixel >> 8) & 0xff);
                average_blue += (unsigned long)((temp_color.pixel >> 0) & 0xff);
            }
        }

        // get hte average color
        unsigned long total_pixels = (unsigned long)display_screen->width * (unsigned long)display_screen->height;
        average_red /= total_pixels;
        average_green /= total_pixels;
        average_blue /= total_pixels;

        // we want the raw RGB, so convert to HSV and set the saturation and value to max before converting back
        RGBColor rgb_raw = {(float)average_red, (float)average_green, (float)average_blue};
        RGBColor rgb_hue;
        HSVColor hsv;
        RGBtoHSV(&rgb_raw, &hsv);
        hsv.S = 1.0f;
        hsv.V = 1.0f;
        HSVtoRGB(&hsv, &rgb_hue);

        pthread_mutex_lock(&s_active_rgb_lock);
        {
            memcpy(&s_active_color_rgb, &rgb_hue, sizeof(RGBColor));
        }
        pthread_mutex_unlock(&s_active_rgb_lock);
        nanosleep(&sleep_time, NULL);
    }
    XDestroyImage(image);
    pthread_exit(NULL);
}

void screen_run(KeyboardLEDState *led_state)
{

    // pull a cache of the shared data
    RGBColor active_rgb_cache;
    pthread_mutex_lock(&s_active_rgb_lock);
    {
        memcpy(&active_rgb_cache, &s_active_color_rgb, sizeof(RGBColor));
    }
    pthread_mutex_unlock(&s_active_rgb_lock);

    // calculate the kb color with smoothing
    s_prev_color_rgb = s_cur_color_rgb;
    s_cur_color_rgb.R = LERP(s_cur_color_rgb.R, active_rgb_cache.R, s_config.m_LerpSmooth * 0.05f);
    s_cur_color_rgb.G = LERP(s_cur_color_rgb.G, active_rgb_cache.G, s_config.m_LerpSmooth * 0.05f);
    s_cur_color_rgb.B = LERP(s_cur_color_rgb.B, active_rgb_cache.B, s_config.m_LerpSmooth * 0.05f);

    // display
    static char buffer[7];
    snprintf(buffer, 7, "%02X%02X%02X", (int)(s_cur_color_rgb.R * 255.0f), (int)(s_cur_color_rgb.G * 255.0f), (int)(s_cur_color_rgb.B * 255.0f));
    memcpy(led_state->m_Color.raw, buffer, 6);

    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = s_config.m_SleepTimeMS * 1000;
    nanosleep(&sleep_time, NULL);
}

void screen_setup(KeyboardLEDState *led_state, ScreenConfig *config)
{
    if (config)
        s_config = *config;

    s_is_thread_running = 1;

    assert(pthread_mutex_init(&s_is_running_lock, NULL) == 0);
    assert(pthread_mutex_init(&s_active_rgb_lock, NULL) == 0);
    pthread_create(&s_thread_id, NULL, &thread_run, NULL);
}

void screen_shutdown()
{
    pthread_mutex_lock(&s_is_running_lock);
    {
        s_is_thread_running = 0;
    }
    pthread_mutex_unlock(&s_is_running_lock);

    pthread_join(s_thread_id, NULL);
    pthread_mutex_destroy(&s_active_rgb_lock);
    pthread_mutex_destroy(&s_is_running_lock);
}