#include "kbd_audio.h"

#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define M_PI 3.14159265358979323846
#define BUFSIZE 1024

static AudioConfig s_config = {1.0f, 2000};
static pa_sample_spec s_sample_spec;
static pa_simple* s_connection;
static float s_buffer[BUFSIZE];
static float s_level;

float audio_get_level()
{
    return s_level;
}

static void get_sound_level(const void *data, size_t size, float time_step, float* out_level)
{
    const float alpha = 1.0 - expf( (-2.0 * M_PI) / (time_step * (float)s_sample_spec.rate) );

    float* sample_data = (float*)data;
    uint32_t total_samples = size / sizeof(float);

    // get the envelope as our sound level 
    // https://en.wikipedia.org/wiki/Envelope_detector
    for(uint32_t sample_index = 0; sample_index < total_samples; ++sample_index)
    {
        const float sample = sample_data[sample_index];        
        const float input_signal = fabsf(sample);
        *out_level = *out_level + alpha * (input_signal - *out_level);
    }
}

void audio_run(KeyboardLEDState* led_state)
{
    // read a chunk of sample data
    int error;
    if (pa_simple_read(s_connection, s_buffer, sizeof(s_buffer), &error) < 0)
    {
        fprintf(stderr, __FILE__ ": pa_simple_read() failed: %s\n", pa_strerror(error));
    }
    else
    {
        // process the sample data
        get_sound_level(s_buffer, sizeof(s_buffer), 0.001f, &s_level);

        float scale = fmin(1.0f, s_level * s_config.m_Sensitivity);
        led_state->m_Brightness.value = (unsigned char)(scale * (float)led_state->m_MaxBrightness.value);
    }    
}

void audio_setup(KeyboardLEDState* led_state, AudioConfig* config)
{
    if (config)
        s_config = *config;

    /* The sample type to use */
    s_sample_spec.format = PA_SAMPLE_FLOAT32LE;
    s_sample_spec.rate = 44100;
    s_sample_spec.channels = 2;


    /* Create the recording stream */
    int error;
    s_connection = pa_simple_new(NULL, "kbd_ligher", PA_STREAM_RECORD, NULL, "record", &s_sample_spec, NULL, NULL, &error);
    if (s_connection == NULL)
    {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n", pa_strerror(error));
    }
    
    s_level = 0;
}

void audio_shutdown()
{
    if (s_connection)
    {
        pa_simple_free(s_connection);
    }
}
