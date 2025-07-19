#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "hacking.h"
#include "notes.h"

void usage(char* prog_name) {
	printf("Usage: %s written by %s\n"
			"\tw\t<data to add to %s>\n"
			"\tr\tread notes file for current user\n"
			"\ts\t<search string to search in notes\n", prog_name, AUTHOR, NOTE_FILENAME);
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

void write_note(const char* note) {
	FILE* notes_file;
	char *buffer, *datafile, *user_tag;

	buffer = (char*) ec_malloc(124);
	datafile = (char*) ec_malloc(124);
	user_tag = (char*) ec_malloc(8);
	strcpy(datafile, NOTE_FILENAME);

	sprintf(user_tag, "%d\t", getuid());
	sprintf(buffer, "%s\n", note);

	notes_file = fopen(datafile, "a");
	if (notes_file == NULL)
		fatal("in write_notes() whlie opening file");
	if (fchmod(fileno(notes_file), S_IRUSR|S_IWUSR) != 0) {
		fatal("in write_note() setting notes_file permissions");
	};
	if (fwrite(user_tag, 1, strlen(user_tag), notes_file) == -1)
		fatal("in write_notes() while writing user_tag to file");
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
	int user_id, note_user_id, printing=1;
	FILE* notes_file;
	char* note_user_tag = (char*) ec_malloc(8);
	char *buffer = (char*) ec_malloc(124);

	user_id = getuid();
	notes_file = fopen(NOTE_FILENAME, "r");
	if (notes_file == NULL)
		fatal("in read_notes() while opening file for reading");

	printf("-----[start of note data]-----\n");
	while (true) {
		fgets(buffer, 124, notes_file);
		if (feof(notes_file) != 0)
			break;
		if (ferror(notes_file) != 0)
			fatal("in read_notes while reading from notes_file to buffer");
		int index_tab_char = strchr(buffer, '\t') - buffer;
		strncpy(note_user_tag, buffer, index_tab_char);
		note_user_id = atoi(note_user_tag);
		if (note_user_id == user_id) {
			printf("%s", buffer + index_tab_char + 1);
		}
	}
	printf("-----[end   of note data]-----\n");

	if (fclose(notes_file) == EOF)
		fatal("in read_notes() while closing file");

	return 0;
}

int search_note(char *note, char *keyword) {
	int keyword_length, match=0;

	keyword_length = strlen(keyword);
	if (keyword_length == 0)
		return -1;

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
			return 1;
	}
	return 0;
}

