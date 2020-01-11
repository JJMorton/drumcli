#ifndef TRACK_H
#define TRACK_H

#include <stdbool.h>

#include "sample.h"
#include "list.h"

typedef enum
{
	NOTE_FULL = 1,
	NOTE_HALF = 2,
	NOTE_THIRD = 3,
	NOTE_QUARTER = 4,
	NOTE_SIXTH = 6,
	NOTE_EIGHTH = 8
} NoteType;

typedef struct
{
	float position;
} Note;

typedef struct
{
	Sample *sample;

	// Times of notes, starting at zero for the first beat
	Node *notes;
	// If nexttoplay == NULL, this indicates that the end of the loop was reached and
	// that we are waiting for it to start again, at which point nexttoplay will
	// be set to the first note in track_shouldplay
	Node *nexttoplay;
	float lastbeatindex;

	// Total number of beats in each loop
	int beatcount;
} Track;

Track *track_create(const char *samplepath, int beatcount);
/*
 * index: the note to play
 * divisions: the number of notes per beat (e.g. quarter-notes would be 4)
 * bpm: beats per minute
 */
void track_addnote(Track *track, float beatindex);
void track_removenote(Track *track, float beatindex);
void track_togglenote(Track *track, float beatindex);
void track_print(Track *track, int divisions);
void track_setlength(Track *track, int beatcount);
bool track_shouldplay(Track *track, float beatindex);
void track_play(Track *track, float beatindex);
void track_free(Track *track);

#endif // TRACK_H

