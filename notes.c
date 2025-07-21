#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "notes.h"

// TODO: support long note buffers

void usage(char* prog_name) {
	printf("Usage: %s written by %s\n"
			"\tw\t<data to add to %s>\n"
			"\td\tremove note\n"
			"\tr\tread notes file for current user\n"
			"\ts\t<search string to search in notes>\n", prog_name, AUTHOR, NOTE_FILENAME);
	exit(0);
}

int main(int argc, char* argv[]) {
	Notes* notes = init_notes();

	if (argc == 2) {
		switch (argv[1][0]) {
		case 'r':
			read_notes(notes, "");
			print_notes(notes);
			break;
		case 'd':
			read_notes(notes, "");
			remove_note(notes);
			break;
		}
	} else if (argc == 3) {
		switch (argv[1][0]) {
		case 's':
			read_notes(notes, argv[2]);
			print_notes(notes);
			break;
		case 'w':
			write_note(argv[2]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	} else {
		usage(argv[0]);
	}

	free_notes(notes);
	return 0;
}

void fatal(const char* message) {
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
		fatal("in ec_fopen() while opening file");

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
	if (fchmod(fileno(notes_file), S_IRUSR|S_IWUSR) != 0)
		fatal("in write_note() setting notes_file permissions");
	if (fwrite(user_tag, 1, strlen(user_tag), notes_file) == -1)
		fatal("in write_note() while writing user_tag to file");
	if (fwrite(timestamp, 1, strlen(timestamp), notes_file) == -1)
		fatal("in write_note() while writing timestamp to file");
	if (fwrite(buffer, 1, strlen(buffer), notes_file) == -1)
		fatal("in write_note() while writing buffer to file");
	if (fclose(notes_file) == EOF)
		fatal("in write_note() while closing file");

	printf("Note has been saved\n");
	free(buffer);
	free(datafile);
	free(timestamp);
	free(user_tag);
}

// serach string is optional, "" if none required
void read_notes(Notes* notes, const char* search_string) {
	int user_id, note_user_id;
	FILE* notes_file;
	char* note = (char*) ec_malloc(128);

	user_id = getuid();
	notes_file = ec_fopen(NOTE_FILENAME, "r");

	while (fgets(note, 128, notes_file)) {
		if (feof(notes_file) != 0)
			break;
		if (ferror(notes_file) != 0)
			fatal("in read_notes() while reading from notes_file to note");

		note_user_id  = get_user_id(note);
		if (note_user_id == user_id && search_note(note, search_string)) {
			add_note(notes, note);
		}
	}
	if (fclose(notes_file) == EOF)
		fatal("in read_notes() while closing file");

	free(note);
}

void print_notes(const Notes* notes) {
	struct tm* note_time;

	printf("-----[start of note data]-----\n");
	for (int i = 0; i < notes->size; i++) {
		note_time = localtime(&notes->raw_times[i]);
		printf("%d. (%02d:%02d %02d/%02d/%02d) %s", i+1, note_time->tm_hour, note_time->tm_min,
				note_time->tm_mday, note_time->tm_mon + 1, (note_time->tm_year + 1900) % 100,
				notes->content[i]);
	}
	printf("-----[end   of note data]-----\n");
}

Notes* init_notes() {
	Notes* notes = ec_malloc(sizeof(Notes));
	notes->size = 0;
	notes->capacity = 124;
	notes->content = ec_malloc(124);
	notes->uids = ec_malloc(124);
	notes->raw_times = ec_malloc(124);

	return notes;
}

void free_notes(Notes* notes) {
    if (notes != NULL) {
		for (int i = 0; i < notes->size; i++)
			free(notes->content[i]);
		free(notes->content);
        free(notes->uids);
        free(notes->raw_times);
        free(notes);
    }
}

void add_note(Notes* notes, const char* note) {
	int index_first_tab = strchr(note, '\t') - note;
	int index_secnd_tab = strchr(note + index_first_tab + 1, '\t') - note;

	if (notes->size >= notes->capacity) {
		notes->capacity *= 2;
		notes->content = realloc(notes->content, notes->capacity);
		notes->uids = realloc(notes->content, notes->capacity);
		notes->raw_times = realloc(notes->content, notes->capacity);
	}

	notes->content[notes->size] = (char*) ec_malloc(128);
	notes->uids[notes->size] = get_user_id(note);
	notes->raw_times[notes->size] = get_note_time(note);
	strncpy(notes->content[notes->size], note + index_secnd_tab + 1, strlen(note));
	notes->size++;
}

void remove_note(Notes* notes) {
	int index_current_note=1, note_uid, user_id = getuid();
	long int index_note_to_del;
	char* note_to_delete = (char*) ec_malloc(128);
	char* line = (char*) ec_malloc(128);
	FILE* notes_file, *temp_file;

	print_notes(notes);
	printf("\nwhich note do you want to remove?\n>> ");
	fgets(note_to_delete, 128, stdin);

	index_note_to_del = strtol(note_to_delete, NULL, 10);
	if (index_note_to_del <= 0 || index_note_to_del > notes->size) {
		fprintf(stderr, "index out of bounds, please choose a note between 1 and %d", notes->size);
		remove_note(notes);
	}

	notes_file = ec_fopen(NOTE_FILENAME, "a");
	temp_file = ec_fopen("temp.txt", "w");
	while (fgets(line, sizeof(line), temp_file)) {
		if (feof(notes_file) != 0)
			break;
		if (ferror(notes_file) != 0)
			fatal("in remove_note() while reading from notes_file to note");

		note_uid = get_user_id(line);
		if (note_uid == user_id && index_current_note++ == index_note_to_del) {
			continue;
		}
		fputs(line, temp_file);
	}

	if (fclose(notes_file) == EOF)
		fatal("in remove_note() while closing file");
	if (fclose(temp_file) == EOF)
		fatal("in remove_note() while closing file");
	unlink(NOTE_FILENAME);
	if (rename("temp.txt", NOTE_FILENAME) != 0)
		fatal("in remove_note() moving file temp.txt to notes_file");

	free(note_to_delete);
	free(line);
	printf("Note has been removed");
}

int get_user_id(const char* note) {
	int index_first_tab = strchr(note, '\t') - note;
	char* note_user_tag = (char*) ec_malloc(64);

	strncpy(note_user_tag, note, index_first_tab);
	int note_user_id = atoi(note_user_tag);

	free(note_user_tag);
	return note_user_id;
}

time_t get_note_time(const char* note) {
	time_t raw_note_time;
	int index_first_tab = strchr(note, '\t') - note;
	int index_secnd_tab = strchr(note + index_first_tab + 1, '\t') - note;
	char* timestamp = (char*) ec_malloc(64);

	strncpy(timestamp, note + index_first_tab + 1, index_secnd_tab - index_first_tab - 1);
	raw_note_time = atoi(timestamp);

	free(timestamp);
	return raw_note_time;
}

bool search_note(const char *note, const char *keyword) {
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

