#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {

	int LINES = 0; 
	int WORDS = 0;
	int CHARACTERS = 0;

	int last_char = '\n';
	int c;

	/* count words through command line input */
	if (argc == 1) {
		while ((c = getchar()) != EOF) {
			CHARACTERS++;
			if (c == '\n')
				LINES++;
			else if (last_char==' ' && c!=' ')
				WORDS++;
			else if (last_char=='\n' && c!=' ')
				WORDS++;
			last_char = c;
		}
	}

	/* count words in file */
	else {
		char *file_name = argv[1];
		FILE *file;
		file = fopen(file_name, "r");

		if (file) {
			while ((c = fgetc(file)) != EOF) {
				CHARACTERS++;
				if (c == '\n')
					LINES++;
				else if (last_char==' ' && c!=' ')
					WORDS++;
				else if (last_char=='\n' && c!=' ')
					WORDS++;
				last_char = c;
			}
			fclose(file);
		}
	}
	printf("%d  ", LINES);
	printf("%d  ", WORDS);
	printf("%d\n", CHARACTERS);

	return 0;
}
