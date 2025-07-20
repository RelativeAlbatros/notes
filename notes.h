#ifndef NOTES_H
#define NOTES_H

#define NOTE_FILENAME "/var/notes"
#define AUTHOR "Rayen Daadaa"

void fatal(char* message);
void* ec_malloc(size_t size);
FILE* ec_fopen(const char* filename, const char* mode);

void write_note(const char* note);
int read_notes(char* search_string);
int get_user_id(char* note_user_tag, char* note);
time_t get_note_time(char* timestamp, char* note);
int search_note(char *note, char *keyword);

#endif
