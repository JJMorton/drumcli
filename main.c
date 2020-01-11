/*
 * TODO:
 *  - Make track_shouldplay more robust (e.g. messes up when changing track length)
 */

#include <stdio.h>
#include <string.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "sample.h"
#include "track.h"

#define FREQUENCY 48000
#define CHANNELS 2
#define CHUNKSIZE 512

#define NUM_TRACKS 16

bool consumeinput(FILE *stream, char *buffer, size_t buffersize);
void printabout(void);
void parsecommand(const char *command, const char *arg);
static int audioThread(void *data);

/* Variables for global states that are modified in parsecommand */
static Track *tracks[NUM_TRACKS] = { NULL };
static int selectedtrack = -1;
static int beatdivisions = 2;
static int BPM = 120;
static int returntomenu = true;

/* The audio thread */
SDL_mutex *tracksmutex;
static bool audio_exit;
static int audioThread(void *data)
{
	while (!audio_exit) {
		int status = SDL_TryLockMutex(tracksmutex);
		if (status == 0)
		{
			float ms_in_beat = 60000.0 / BPM;
			float currentbeat = (float) SDL_GetTicks() / ms_in_beat;
			for (int i = 0; i < NUM_TRACKS; i++)
			{
				Track *track = tracks[i];
				if (track != NULL)
				{
					if (track_shouldplay(track, currentbeat))
					{
						track_play(track, currentbeat);
					}
				}
			}
			SDL_UnlockMutex(tracksmutex);
		}

		SDL_Delay(1);
	}

	return 0;
}

int main(void)
{
	audio_exit = false;
	if (SDL_Init(SDL_INIT_AUDIO) < 0) return 1;
	Mix_OpenAudio(FREQUENCY, AUDIO_U8, CHANNELS, CHUNKSIZE);
	tracksmutex = SDL_CreateMutex();
	SDL_Thread *thread = SDL_CreateThread(audioThread, "audio", NULL);

	const size_t buffersize = 128;
	char buffer[buffersize];
	char command[buffersize];
	char arg[buffersize];

	while (returntomenu)
	{
		printf("DRUMCLI > ");

		// Initialise buffers to zeroes
		memset(buffer, 0, buffersize);
		memset(command, 0, buffersize);
		memset(arg, 0, buffersize);

		// Get the command and argument input
		consumeinput(stdin, buffer, buffersize);
		sscanf(buffer, "%s%s", command, arg);
		
		// Parse the command and execute it
		if (SDL_LockMutex(tracksmutex) == 0)
		{
			parsecommand(command, arg);
			SDL_UnlockMutex(tracksmutex);
		}
		else
		{
			printf("Something went very wrong, failed to handle command\n");
		}

	}

	audio_exit = true;
	SDL_WaitThread(thread, NULL);
	SDL_DestroyMutex(tracksmutex);

	for (int i = 0; i < NUM_TRACKS; i++)
	{
		if (tracks[i] == NULL) continue;
		track_free(tracks[i]);
	}

	Mix_CloseAudio();

	return 0;
}

void printabout(void)
{
	printf(
		" ________  ________  ___  ___  _____ ______   ________  ___       ___     \n"
		"|\\   ___ \\|\\   __  \\|\\  \\|\\  \\|\\   _ \\  _   \\|\\   ____\\|\\  \\     |\\  \\    \n"
		"\\ \\  \\_|\\ \\ \\  \\|\\  \\ \\  \\\\\\  \\ \\  \\\\\\__\\ \\  \\ \\  \\___|\\ \\  \\    \\ \\  \\   \n"
		" \\ \\  \\ \\\\ \\ \\   _  _\\ \\  \\\\\\  \\ \\  \\\\|__| \\  \\ \\  \\    \\ \\  \\    \\ \\  \\  \n"
		"  \\ \\  \\_\\\\ \\ \\  \\\\  \\\\ \\  \\\\\\  \\ \\  \\    \\ \\  \\ \\  \\____\\ \\  \\____\\ \\  \\ \n"
		"   \\ \\_______\\ \\__\\\\ _\\\\ \\_______\\ \\__\\    \\ \\__\\ \\_______\\ \\_______\\ \\__\\\n"
		"    \\|_______|\\|__|\\|__|\\|_______|\\|__|     \\|__|\\|_______|\\|_______|\\|__|\n"
		"\n"
		"drumcli - by JJMorton\n"
		"Make drum beats at the command line!\n"
		"GitHub: https://github.com/JJMorton\n"
	);
}

void printhelp(void)
{
	printf(
		"Sections in square brackets [] are optional\n"
		"Arguments in angle brackets <> are required\n"
		"a[dd]       <SAMPLE PATH> Add a new track\n"
		"r[emove]                  Remove the selected track\n"
		"s[elect]    <TRACK NUM>   Change the selected track\n"
		"t[oggle]    <NOTE NUM>    Toggle a note in the selected track\n"
		"l[ength]    <BEATS>       Set the length of a track in a number of beats\n"
		"d[ivisions] <DIVISIONS>   Set the number of notes per beat\n"
		"p[rint]                   Show all the tracks\n"
		"help                      Show this help message\n"
		"about                     About this program\n"
		"q[uit]                    Exit the program\n"
	);
}

void parsecommand(const char *command, const char *arg)
{
	if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0)
	{
		returntomenu = false;
		printf("Exiting...\n");
	}
	else if (strcmp(command, "help") == 0)
	{
		printhelp();
	}
	else if (strcmp(command, "about") == 0)
	{
		printabout();
	}
	else if (strcmp(command, "add") == 0 || strcmp(command, "a") == 0)
	{
		if (arg[0] == 0)
		{
			printf("Please provide a path to the sample\n");
			return;
		}
		int i;
		bool foundspace = false;
		for (i = 0; i < NUM_TRACKS && !foundspace; i++)
		{
			if (tracks[i] == NULL)
			{
				tracks[i] = track_create(arg, 4);
				printf("Created track %i with sample %s\n", i, arg);
				foundspace = true;
			}
		}
		if (!foundspace)
		{
			printf("You have reached the maximum number of allowed tracks\n");
		}
	}
	else if (strcmp(command, "remove") == 0 || strcmp(command, "r") == 0)
	{
		if (selectedtrack == -1)
		{
			printf("Select a track first\n");
			return;
		}
		track_free(tracks[selectedtrack]);
		tracks[selectedtrack] = NULL;
		printf("Removed track %i\n", selectedtrack);
		selectedtrack = -1;
	}
	else if (strcmp(command, "print") == 0 || strcmp(command, "p") == 0)
	{
		for (int i = 0; i < NUM_TRACKS; i++)
		{
			if (tracks[i] != NULL)
			{
				printf("%c %i ", (selectedtrack == i ? '*' : ' '), i);
				track_print(tracks[i], beatdivisions);
			}
		}
	}
	else if (strcmp(command, "select") == 0 || strcmp(command, "s") == 0)
	{
		if (arg[0] == 0)
		{
			printf("Please provide a track number to select\n");
			return;
		}
		int selection = atoi(arg);
		if (tracks[selection] == NULL)
		{
			printf("Invalid track number\n");
			return;
		}

		selectedtrack = selection;
		printf("Selected track %i\n", selectedtrack);
	}
	else if (strcmp(command, "toggle") == 0 || strcmp(command, "t") == 0)
	{
		if (arg[0] == 0)
		{
			printf("Please provide a note index to toggle\n");
			return;
		}
		if (selectedtrack == -1)
		{
			printf("Select a track first\n");
			return;
		}
		int note = atoi(arg);
		float beatindex = (float) note / beatdivisions;
		if (beatindex < 0 || beatindex >= tracks[selectedtrack]->beatcount)
		{
			printf("Invalid note index\n");
			return;
		}
		track_togglenote(tracks[selectedtrack], beatindex);
		printf("Toggled note %i (beat %g) in track %i\n", note, beatindex, selectedtrack);
	}
	else if (strcmp(command, "length") == 0 || strcmp(command, "l") == 0)
	{
		if (arg[0] == 0)
		{
			printf("Please provide the desired length of the track in beats\n");
			return;
		}
		if (selectedtrack == -1)
		{
			printf("Select a track first\n");
			return;
		}
		int beatcount = atoi(arg);
		if (beatcount < 1)
		{
			printf("Invalid number of beats\n");
			return;
		}
		track_setlength(tracks[selectedtrack], beatcount);
		printf("Set the length of track %i to %i beats\n", selectedtrack, beatcount);
	}
	else if (strcmp(command, "divisions") == 0 || strcmp(command, "d") == 0)
	{
		if (arg[0] == 0)
		{
			printf("Please provide the number of divisions to make per beat\n");
			return;
		}
		int divisions = atoi(arg);
		if (divisions < 1)
		{
			printf("Invalid number of divisions\n");
			return;
		}
		beatdivisions = divisions;
		printf("Set the number of divisions per beat to %i\n", beatdivisions);
	}
	else if (command[0] != 0)
	{
		printf("Invalid command, enter \"help\" for help\n");
	}

}

bool consumeinput(FILE *stream, char *buffer, size_t buffersize)
{
	/*
	 * Consumes the whole line in the input buffer and stores said line in 'buffer'.
	 * Using this instead of a plain scanf ensures that nothing is left in the input buffer afterwards
	 * and that the size of the buffer is not exceeded.
	 * Typical use of this function looks like the following:
	 * 
	 * char *buffer = (char *) malloc(64);
	 * consumeinput(stdin, buffer, 64);
	 * sscanf(buffer, fmt, ...);
	 * free(buffer);
	 *
	 * Returns whether or not the buffer was large enough to store the whole line
	 *
	 * The condition in the for loop used the following as a reference:
	 * https://stackoverflow.com/questions/7898215/how-to-clear-input-buffer-in-c
	 */
	memset(buffer, 0, buffersize); // Fill the buffer with null bytes
	char c;
	int i;
	for (i = 0; (c = getc(stream)) != '\n' && c != EOF; i++)
	{
		// Need to leave a null byte at the end of the buffer
		if (i < buffersize - 1)
		{
			buffer[i] = c;
		}
	}
	return i < buffersize;
}

