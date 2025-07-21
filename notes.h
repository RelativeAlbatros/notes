#ifndef NOTES_H
#define NOTES_H

#define NOTE_FILENAME "/var/notes"
#define AUTHOR "Rayen Daadaa"

typedef struct Notes {
	char** content;
	int capacity;
	int size;
	int* uids;
	time_t* raw_times;
} Notes;

void fatal(const char* message);
void* ec_malloc(const size_t size);
FILE* ec_fopen(const char* filename, const char* mode);

void write_note(const char* note);
void read_notes(Notes* notes, const char* search_string);
void print_notes(const Notes* notes);
Notes* init_notes();
void free_notes(Notes* notes);
void add_note(Notes* notes, const char* note);
void remove_note(Notes* notes);
int get_user_id(const char* note);
time_t get_note_time(const char* note);
bool search_note(const char *note, const char *keyword);

#endif
