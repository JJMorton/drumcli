#include <math.h>

#include "track.h"

Track *track_create(const char *samplepath, int beatcount)
{
	Track *track = malloc(sizeof(Track));
	if (track == NULL)
	{
		printf("Failed to allocate memory for track\n");
		return NULL;
	}

	/* Create sample for playing track */
	track->sample = sample_create(samplepath);
	if (track->sample == NULL)
	{
		free(track);
		return NULL;
	}

	track->notes = NULL;
	track->lastplayed = NULL;
	track->beatcount = beatcount;

	return track;
}

/*
 * index: the note to play
 * divisions: the number of notes per beat (e.g. quarter-notes would be 4)
 * bpm: beats per minute
 */
void track_togglenote(Track *track, int index, int divisions)
{
	// Calculate time that note should be played at
	float position = index * 1.0 / divisions;

	Note *newnote = malloc(sizeof(Note));
	if (newnote == NULL)
	{
		printf("Failed to allocate memory for note\n");
		return;
	}
	newnote->position = position;
	newnote->loopplayed = -1;

	Node *current = track->notes;
	while (current != NULL)
	{
		Note *note = (Note *) current->data;

		if (note->position == position)
		{
			// The exact note was found, remove it
			// Need to make sure that track->lastplayed still references a valid note
			if (track->lastplayed == current)
				track->lastplayed = current->previous;
			track->notes = list_remove(current);
			free(newnote);
			return;
		}
		
		if (note->position > position)
		{
			// We've found the point where the new note should be inserted
			track->notes = list_createbefore(newnote, current);
			return;
		}

		current = current->next;
	}

	// If we reached the end of the list, the new note should be appended to the end
	track->notes = list_createafter(newnote, list_getlast(track->notes));

}

void track_print(Track *track, int divisions)
{
	Node *current = track->notes;
	int numnotes = track->beatcount * divisions;
	char output[numnotes + 1];
	memset(output, '-', numnotes);
	output[numnotes] = '\0';

	int hiddencount = 0;

	while (current != NULL)
	{
		Note *note = (Note *) current->data;
		if (note != NULL)
		{
			float noteindex = note->position * divisions;
			if (floorf(noteindex) == noteindex && noteindex < numnotes)
				output[(int) noteindex] = '#';
			else
				hiddencount++;
		}
		current = current->next;
	}

	if (hiddencount > 0)
		printf("[ %s ] (%i not shown)\n", output, hiddencount);
	else
		printf("[ %s ]\n", output);
}

bool track_shouldplay(Track *track, float beatindex)
{
	if (track->notes == NULL) return false;

	if (track->lastplayed == NULL)
	{
		track->lastplayed = list_getlast(track->notes);
	}

	Note *nextnote;
	if (track->lastplayed->next == NULL)
	{
		nextnote = (Note *) track->notes->data;
	}
	else
	{
		nextnote = (Note *) track->lastplayed->next->data;
	}

	return beatindex >= nextnote->position + track->beatcount * (nextnote->loopplayed + 1);
}

void track_play(Track *track, float beatindex)
{
	int loopno = floorf(beatindex / (float) track->beatcount);
	sample_play(track->sample);
	track->lastplayed = track->lastplayed->next;
	if (track->lastplayed == NULL)
	{
		track->lastplayed = track->notes;
	}
	Note *note = (Note *) track->lastplayed->data;
	note->loopplayed = loopno;
}

void track_free(Track *track)
{
	if (track == NULL) return;
	sample_free(track->sample);
	list_free(track->notes);
	free(track);
}

