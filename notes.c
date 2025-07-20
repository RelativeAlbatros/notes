#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "notes.h"

void usage(char* prog_name) {
	printf("Usage: %s written by %s\n"
			"\tw\t<data to add to %s>\n"
			"\tr\tread notes file for current user\n"
			"\ts\t<search string to search in notes>\n", prog_name, AUTHOR, NOTE_FILENAME);
	exit(0);
}

int main(int argc, char* argv[]) {
	if (argc == 2 && argv[1][0] == 'r') {
		read_notes("");
	} else if (argc == 3) {
		switch (argv[1][0]) {
		case 's':
			read_notes(argv[2]);
			break;
		case 'w':
			write_note(argv[2]);
			break;
		}
	} else {
		usage(argv[0]);
	}

	return 0;
}

void fatal(char* message) {
	char error_message[100];

	strcpy(error_message, "[!!] Fatal Error ");
	strncat(error_message, message, 83);
	perror(error_message);
	exit(-1);
}

void* ec_malloc(size_t size) {
	void *ptr;
	ptr = malloc(size);
	if (ptr == NULL) {
		fatal("in ec_malloc() on memory allocation");
	}
	return ptr;
}

FILE* ec_fopen(const char* filename, const char* mode) {
	FILE* file = fopen(filename, mode);
	if (file == NULL)
		fatal("in write_notes() whlie opening file");

	return file;
}

void write_note(const char* note) {
	FILE* notes_file;
	char *buffer, *datafile, *user_tag, *timestamp;

	buffer = (char*) ec_malloc(128);
	datafile = (char*) ec_malloc(128);
	user_tag = (char*) ec_malloc(8);
	timestamp = (char*) ec_malloc(64);
	strcpy(datafile, NOTE_FILENAME);

	sprintf(user_tag, "%d\t", getuid());
	sprintf(timestamp, "%d\t", time(NULL));
	snprintf(buffer, 128, "%s\n", note);

	notes_file = ec_fopen(NOTE_FILENAME, "a");
	if (fchmod(fileno(notes_file), S_IRUSR|S_IWUSR) != 0) {
		fatal("in write_note() setting notes_file permissions");
	};
	if (fwrite(user_tag, 1, strlen(user_tag), notes_file) == -1)
		fatal("in write_notes() while writing user_tag to file");
	if (fwrite(timestamp, 1, strlen(timestamp), notes_file) == -1)
		fatal("in write_notes() while writing timestamp to file");
	if (fwrite(buffer, 1, strlen(buffer), notes_file) == -1)
		fatal("in write_notes() while writing buffer to file");
	if (fclose(notes_file) == -1)
		fatal("in write_notes() while closing file");

	printf("Note has been saved.\n");
	free(buffer);
	free(datafile);
}

// serach string is optional, "" if none required
int read_notes(char* search_string) {
	int user_id, note_user_id;
	time_t epoch_note_time;
	struct tm* note_time;
	FILE* notes_file;

	char* note_user_tag = (char*) ec_malloc(8);
	char* note = (char*) ec_malloc(128);
	char* timestamp = (char*) ec_malloc(64);

	user_id = getuid();
	notes_file = ec_fopen(NOTE_FILENAME, "r");

	printf("-----[start of note data]-----\n");
	while (true) {
		fgets(note, 128, notes_file);

		if (feof(notes_file) != 0)
			break;
		if (ferror(notes_file) != 0)
			fatal("in read_notes while reading from notes_file to note");

		int index_first_tab = strchr(note, '\t') - note;
		int index_secnd_tab = strchr(note + index_first_tab + 1, '\t') - note;

		note_user_id  = get_user_id(note_user_tag, note);
		epoch_note_time = get_note_time(timestamp, note);
		note_time = localtime(&epoch_note_time);

		if (note_user_id == user_id && search_note(note, search_string)) {
			printf("(%02d:%02d %02d/%02d/%02d) %s", note_time->tm_hour, note_time->tm_min,
					note_time->tm_mday, note_time->tm_mon + 1, (note_time->tm_year + 1900) % 100,
					note + index_secnd_tab + 1);
		}
	}
	printf("-----[end   of note data]-----\n");

	if (fclose(notes_file) == EOF)
		fatal("in read_notes() while closing file");

	return 0;
}

// sets note_user_tag and returns user id of note
int get_user_id(char* note_user_tag, char* note) {
	int index_first_tab = strchr(note, '\t') - note;

	strncpy(note_user_tag, note, index_first_tab);
	int note_user_id = atoi(note_user_tag);

	return note_user_id;
}

// sets timestamp from note, note_time from  and returns epoch_note_time
time_t get_note_time(char* timestamp, char* note) {
	time_t epoch_note_time;
	int index_first_tab = strchr(note, '\t') - note;
	int index_secnd_tab = strchr(note + index_first_tab + 1, '\t') - note;

	strncpy(timestamp, note + index_first_tab + 1, index_secnd_tab - index_first_tab - 1);
	epoch_note_time = atoi(timestamp);

	return epoch_note_time;
}

bool search_note(char *note, char *keyword) {
	int keyword_length, match=0;

	keyword_length = strlen(keyword);
	if (keyword_length == 0)
		return true;

	for (int i = 0; i < strlen(note); i++) {
		if (note[i] == keyword[match])
			match++;
		else {
			if (note[i] == keyword[0])
				match = 1;
			else
				match = 0;
		}
		if (match == keyword_length)
			return true;
	}
	return false;
}

