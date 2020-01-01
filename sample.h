#ifndef SAMPLE_H
#define SAMPLE_H

#include <stdbool.h>

#include <SDL.h>
#include <SDL_mixer.h>

typedef struct
{
	int chan;
	Mix_Chunk *chunk;
} Sample;

Sample *sample_create(const char *filepath);
void sample_play(Sample *sample);
bool sample_isplaying(Sample *sample);
void sample_free(Sample *sample);

#endif // SAMPLE_H
