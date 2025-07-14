// Abstract-Shader-Engine - Ougi Washi

#include "as_audio.h"
#include <stdlib.h>
#include <stdio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024

typedef struct {
    as_vec3 amps;
    
    // internal
    b8 running;
    float* buffer;
    PaStream* stream;
    int buffer_size;
} as_audio_data_t;
static as_audio_data_t as_audio_data = {0};

// Simple zero-crossing frequency detection
float detect_frequency(float* buffer, int size, float sample_rate) {
    int zero_crossings = 0;
    
    for (int i = 1; i < size; i++) {
        if ((buffer[i-1] >= 0 && buffer[i] < 0) || (buffer[i-1] < 0 && buffer[i] >= 0)) {
            zero_crossings++;
        }
    }
    
    // Frequency is roughly zero crossings per second divided by 2
    float frequency = (zero_crossings * sample_rate) / (2.0f * size);
    return frequency;
}

// Calculate RMS volume
float calculate_volume(float* buffer, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += buffer[i] * buffer[i];
    }
    return sqrt(sum / size);
}

// Simple frequency band classification
void classify_frequency_bands(float frequency, float volume, float* low, float* mid, float* high) {
    *low = 0.0f;
    *mid = 0.0f;
    *high = 0.0f;
    
    if (frequency >= 20 && frequency <= 3000) {
        *low = volume;
    } else if (frequency > 3000 && frequency <= 4000) {
        *mid = volume;
    } else if (frequency > 4000 && frequency <= 20000) {
        *high = volume;
    }
}


// Audio callback function
static int as_audio_callback(const void* input_buffer, void* output_buffer,
                         unsigned long frames_per_buffer,
                         const PaStreamCallbackTimeInfo* time_info,
                         PaStreamCallbackFlags status_flags,
                         void* user_data) {
    
    as_audio_data_t* data = (as_audio_data_t*)user_data;
    const float* input = (const float*)input_buffer;
    
    if (input_buffer == NULL) {
        return paContinue;
    }
    
    // Copy input to buffer
    for (unsigned long i = 0; i < frames_per_buffer; i++) {
        data->buffer[i] = input[i];
    }
    
    float volume = calculate_volume(data->buffer, frames_per_buffer);
    float frequency = detect_frequency(data->buffer, frames_per_buffer, SAMPLE_RATE);
    
    float low_vol, mid_vol, high_vol;
    classify_frequency_bands(frequency, volume, &low_vol, &mid_vol, &high_vol);
    
    float volume_percent = volume * 10000.0f; 
    if (volume_percent > 100.0f) volume_percent = 100.0f;
    
    // Scale band volumes
    float low_percent = low_vol * 10000.0f;
    float mid_percent = mid_vol * 10000.0f;
    float high_percent = high_vol * 10000.0f;
    
    if (low_percent > 100.0f) low_percent = 100.0f;
    if (mid_percent > 100.0f) mid_percent = 100.0f;
    if (high_percent > 100.0f) high_percent = 100.0f;
    
    data->amps = (as_vec3){low_vol, mid_vol, high_vol};
    return paContinue;
}

void list_audio_devices() {
    int num_devices = Pa_GetDeviceCount();
    printf("Available audio devices:\n");
    
    for (int i = 0; i < num_devices; i++) {
        const PaDeviceInfo* device_info = Pa_GetDeviceInfo(i);
        const PaHostApiInfo* host_api_info = Pa_GetHostApiInfo(device_info->hostApi);
        
        printf("Device %d: %s (Host API: %s)\n", i, device_info->name, host_api_info->name);
        printf("  Max input channels: %d\n", device_info->maxInputChannels);
        printf("  Max output channels: %d\n", device_info->maxOutputChannels);
        printf("  Default sample rate: %.0f Hz\n", device_info->defaultSampleRate);
        printf("\n");
    }
}

void as_audio_init() {
    list_audio_devices();
    PaStreamParameters input_parameters;
    PaError err;
    
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        as_audio_cleanup();
        return;
    }
    
    as_audio_data.buffer = (float*)malloc(FRAMES_PER_BUFFER * sizeof(float));
    as_audio_data.buffer_size = FRAMES_PER_BUFFER;
    
    if (!as_audio_data.buffer) {
        printf("Memory allocation failed\n");
        as_audio_cleanup();
        return;
    }
    
    int input_device = Pa_GetDefaultInputDevice();
    if (input_device == paNoDevice) {
        printf("No input device found\n");
        as_audio_cleanup();
        return;
    }
    
    input_parameters.device = input_device;
    input_parameters.channelCount = 1;
    input_parameters.sampleFormat = paFloat32;
    input_parameters.suggestedLatency = Pa_GetDeviceInfo(input_device)->defaultLowInputLatency;
    input_parameters.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(&as_audio_data.stream,
                        &input_parameters,
                        NULL,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,
                        as_audio_callback,
                        &as_audio_data);
    
    if (err != paNoError) {
        printf("Cannot open stream: %s\n", Pa_GetErrorText(err));
        as_audio_cleanup();
        return;
    }
    
    err = Pa_StartStream(as_audio_data.stream);
    if (err != paNoError) {
        printf("Cannot start stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(as_audio_data.stream);
        as_audio_cleanup();
        return;
    }
    
    printf("Volume and frequency detection active...\n\n");
    as_audio_data.running = true;
}

void as_audio_cleanup(){
    as_audio_data.running = false;
    PaError err;
    err = Pa_StopStream(as_audio_data.stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        goto cleanup;
    }

    err = Pa_CloseStream(as_audio_data.stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

cleanup:
    free(as_audio_data.buffer);
    Pa_Terminate();
}


as_vec3 as_audio_get_amplitudes(){
    return as_audio_data.amps;
}

