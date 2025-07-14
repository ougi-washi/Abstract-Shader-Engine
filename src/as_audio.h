// Abstract-Shader-Engine - Ougi Washi

#ifndef AS_AUDIO_H
#define AS_AUDIO_H

#include "as_types.h"
#include <portaudio.h>

void as_audio_init();
void as_audio_cleanup();
as_vec3 as_audio_get_amplitudes();

#endif // AS_AUDIO_H
