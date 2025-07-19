debug:
	gcc notes.c -g -o notes

all:
	gcc notes.c -o notes

install: all
	chown root:root notes
	chmod u+s notes
	mv notes ${DESTDIR}${PREFIX}/bin/notes

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/notes
