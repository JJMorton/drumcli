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
	track->nexttoplay = NULL;
	track->lastbeatindex = 0;
	track->beatcount = beatcount;

	return track;
}

void track_addnote(Track *track, float beatindex)
{
	Note *newnote = malloc(sizeof(Note));
	if (newnote == NULL)
	{
		printf("Failed to allocate memory for note\n");
		return;
	}
	newnote->position = beatindex;

	Node *current = track->notes;
	while (current != NULL)
	{
		Note *note = (Note *) current->data;

		if (note->position > beatindex)
		{
			// We've found the point where the new note should be inserted
			track->notes = list_createbefore(newnote, current);
			if (track->nexttoplay == NULL) track->nexttoplay = track->notes;
			return;
		}

		current = current->next;
	}

	// If we reached the end of the list without finding the insertion
	// point, the new note should be appended to the end
	track->notes = list_createafter(newnote, list_getlast(track->notes));
	if (track->nexttoplay == NULL) track->nexttoplay = track->notes;
}

void track_removenote(Track *track, float beatindex)
{
	Node *current = track->notes;
	while (current != NULL)
	{
		Note *note = (Note *) current->data;

		if (note->position == beatindex)
		{
			// The note was found, remove it
			// Need to make sure that track->nexttoplay still references a valid note
			if (track->nexttoplay == current)
				track->nexttoplay = track->nexttoplay->next;
			track->notes = list_remove(current);
			return;
		}

		current = current->next;
	}
}

void track_togglenote(Track *track, float beatindex)
{
	// This function is O(n^2) but it doesn't matter because there are never going
	// to be so many notes that it will take a considerable amount of time.
	// Writing separate functions for adding and removing notes gives clearer
	// code which is a worthy sacrifice in this case.

	Node *current = track->notes;
	while (current != NULL)
	{
		Note *note = (Note *) current->data;

		if (note->position == beatindex)
		{
			// The exact note was found, remove it
			track_removenote(track, beatindex);
			return;
		}
		
		if (note->position > beatindex)
		{
			// We've found the point where the new note should be inserted
			track_addnote(track, beatindex);
			return;
		}

		current = current->next;
	}

	// If we reached the end of the list, the new note should be appended to the end
	track_addnote(track, beatindex);
}

void track_setlength(Track *track, int beatcount)
{
	// Sets the length of the track and removes notes outside of the new range
	track->beatcount = beatcount;
	Node *current = track->notes;
	while (current != NULL)
	{
		Note *n = (Note *) current->data;
		current = current->next;
		if (n->position >= beatcount)
		{
			track_removenote(track, n->position);
		}
	}
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

bool track_shouldplay(Track *track, float beat)
{
	// If there are no notes in the tracks, obviously it shouldn't play
	if (track->notes == NULL) return false;

	float beatindex = fmodf(beat, track->beatcount);

	// If the loop has finished playing, wait until we begin the loop again
	float lastbeatindex = track->lastbeatindex;
	track->lastbeatindex = beatindex;
	if (track->nexttoplay == NULL)
	{
		if (beatindex < lastbeatindex)
			track->nexttoplay = track->notes;
		else
			return false;
	}

	Note *note = (Note *) track->nexttoplay->data;
	return beatindex >= note->position;
}

void track_play(Track *track, float beatindex)
{
	sample_play(track->sample);
	track->nexttoplay = track->nexttoplay->next;
}

void track_free(Track *track)
{
	if (track == NULL) return;
	sample_free(track->sample);
	list_free(track->notes);
	free(track);
}

