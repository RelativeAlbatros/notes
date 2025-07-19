#ifndef NOTES_H
#define NOTES_H

#define NOTE_FILENAME "/var/notes"
#define AUTHOR "Rayen Daadaa"

void write_note(const char* note);
int read_notes(char* search_string);
int print_notes(int fd, int uid, char* search_string);
int find_user_note(int fd, int user_id);
int search_note(char *note, char *keyword);

#endif
