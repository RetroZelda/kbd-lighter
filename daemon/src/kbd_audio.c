#include "kbd_audio.h"

#include "utility/audio_parametric_equalizer.h"

#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define M_PI 3.14159265358979323846
#define BUFSIZE 1024

static AudioConfig s_config = {0.0f, 1.0f, 1.0f, 2000};
static pa_sample_spec s_sample_spec;
static pa_simple *s_connection;
static float s_buffer[BUFSIZE];
static float s_level;

#define SAMPLE_RATE 44100
#define NUM_SEGMENTS 10
static APE_EqualizerHandle s_FrequencyHandles[NUM_SEGMENTS];
static APE_FrequencySpectrum s_FrequencySegments[NUM_SEGMENTS] = 
{
    {SAMPLE_RATE, 31.0f,    17.0f,   5.0f, 0.0f, 0.0f},
    {SAMPLE_RATE, 62.0f,    31.0f,   5.0f, 0.0f, 4.0f},
    {SAMPLE_RATE, 125.0f,   75.0f,   5.0f, 0.0f, 8.0f},
    {SAMPLE_RATE, 250.0f,   125.0f,  5.0f, 0.0f, 0.0f},
    {SAMPLE_RATE, 500.0f,   250.0f,  5.0f, 0.0f, -5.0f},
    {SAMPLE_RATE, 1000.0f,  500.0f,  5.0f, 0.0f, -5.0f},
    {SAMPLE_RATE, 2000.0f,  750.0f,  5.0f, 0.0f, -5.0f},
    {SAMPLE_RATE, 4000.0f,  1500.0f, 5.0f, 0.0f, -5.0f},
    {SAMPLE_RATE, 8000.0f,  3000.0f, 5.0f, 0.0f, -5.0f},
    {SAMPLE_RATE, 16000.0f, 6000.0f, 5.0f, 0.0f, -5.0f} 
};

float audio_get_level()
{
    return s_level;
}

void low_pass_filter(const void *const in_data, const void *out_data, size_t data_size, double cutoff_frequency)
{
    const float *const in_samples = (const float *const)in_data;
    float *out_samples = (float *)out_data;
    uint32_t total_samples = data_size / sizeof(float);
    out_samples[0] = in_samples[0];

    double RC = 1.0 / (cutoff_frequency * 2.0 * M_PI);
    double dt = 1.0 / s_sample_spec.rate;
    double alpha = dt / (RC + dt);
    for (int i = 1; i < total_samples; ++i)
    {
        out_samples[i] = out_samples[i - 1] + (alpha * (in_samples[i] - out_samples[i - 1]));
    }
}

static void get_sound_level(const void *data, size_t size, float time_step, float *out_level)
{
    // static float filtered_buffer[BUFSIZE];
    // uint32_t total_samples = size / sizeof(float);
    // low_pass_filter(data, filtered_buffer, size, 25000.0);
    // float *sample_data = (float *)filtered_buffer;


    static float filtered_buffer[2][BUFSIZE];
    uint32_t total_samples = size / sizeof(float);

    uint8_t toggler = 0;
    APE_Sample* source_buffer = (APE_Sample*)data;
    APE_Sample* destination_buffer = filtered_buffer[0];
    for(uint32_t segment_pass = 0; segment_pass < NUM_SEGMENTS; ++segment_pass)
    {
        ape_run_filter(s_FrequencyHandles[segment_pass], &s_FrequencySegments[segment_pass], source_buffer, destination_buffer, total_samples);
    
        if(toggler ^= 1)
        {
            source_buffer = filtered_buffer[0];
            destination_buffer = filtered_buffer[1];
        }
        else
        {
            source_buffer = filtered_buffer[1];
            destination_buffer = filtered_buffer[0];
        }        
    }
    //low_pass_filter(source_buffer, destination_buffer, size, 100.0);
    APE_Sample *sample_data = destination_buffer;

    // get the envelope as our sound level
    // https://en.wikipedia.org/wiki/Envelope_detector
    const float alpha = 1.0 - expf((-2.0 * M_PI) / (time_step * (float)s_sample_spec.rate));
    for (uint32_t sample_index = 0; sample_index < total_samples; ++sample_index)
    {
        const float sample = sample_data[sample_index];
        const float input_signal = fabsf(sample);
        *out_level = *out_level + alpha * (input_signal - *out_level);
    }
}

void audio_run(KeyboardLEDState *led_state)
{
    // read a chunk of sample data
    int error;
    if (pa_simple_flush(s_connection, &error) < 0)
    {
        fprintf(stderr, __FILE__ ": pa_simple_flush() failed: %s\n", pa_strerror(error));
    }
    if (pa_simple_read(s_connection, s_buffer, sizeof(s_buffer), &error) < 0)
    {
        fprintf(stderr, __FILE__ ": pa_simple_read() failed: %s\n", pa_strerror(error));
    }
    else
    {
        // process the sample data
        get_sound_level(s_buffer, sizeof(s_buffer), 1.0f / (float)SAMPLE_RATE, &s_level);

        float scale = pow(s_level * s_level, 1.0f / s_config.m_Sensitivity);
        scale = LERP(s_config.m_Min, 1.0f + (1.0f - s_config.m_Max), scale);
        led_state->m_Brightness.value = (unsigned char)(scale * (float)led_state->m_MaxBrightness.value);
    }
}

void audio_setup(KeyboardLEDState *led_state, AudioConfig *config)
{
    if (config)
        s_config = *config;

    /* The sample type to use */
    s_sample_spec.format = PA_SAMPLE_FLOAT32LE;
    s_sample_spec.rate = SAMPLE_RATE;
    s_sample_spec.channels = 1;

    /* Create the recording stream */
    int error;
    s_connection = pa_simple_new(NULL, "kbd_ligher", PA_STREAM_RECORD, NULL, "record", &s_sample_spec, NULL, NULL, &error);
    if (s_connection == NULL)
    {
        fprintf(stderr, __FILE__ ": pa_simple_new() failed: %s\n", pa_strerror(error));
    }

    for(uint32_t segment_pass = 0; segment_pass < NUM_SEGMENTS; ++segment_pass)
    {
        s_FrequencyHandles[segment_pass] = ape_obtain();
    }

    s_level = 0;
}

void audio_shutdown()
{
    if (s_connection)
    {
        pa_simple_free(s_connection);
    }

    for(uint32_t segment_pass = 0; segment_pass < NUM_SEGMENTS; ++segment_pass)
    {
        ape_return(s_FrequencyHandles[segment_pass]);
    }
}
