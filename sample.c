#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "sample.h"

Sample *sample_create(const char *filepath)
{
	Sample *sample = malloc(sizeof(Sample));
	if (sample == NULL)
	{
		printf("Failed to allocate memory for sample\n");
		return NULL;
	}

	sample->chunk = Mix_LoadWAV(filepath);
	if (sample->chunk == NULL)
	{
		printf("Failed to load WAV file '%s'\n", filepath);
		free(sample);
		return NULL;
	}

	int numchannels = Mix_AllocateChannels(-1);
	sample->chan = numchannels;
	Mix_AllocateChannels(numchannels + 1);

	return sample;
}

void sample_play(Sample *sample)
{
	Mix_PlayChannel(sample->chan, sample->chunk, 0);
}

bool sample_isplaying(Sample *sample)
{
	return Mix_Playing(sample->chan);
}

void sample_free(Sample *sample)
{
	if (sample == NULL) return;
	Mix_FreeChunk(sample->chunk);
	sample->chunk = NULL;
	free(sample);
}

