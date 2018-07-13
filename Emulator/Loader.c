#include "stdio.h"
#include "string.h"
#include "stdlib.h"


FILE *xmefile; // the X Makina executable file
FILE *lisfile; // .lis file for 1st and 2nd pass errors
unsigned short lc; // location counter

int newFileFromOld(FILE **newfptr, char *oldFileName, char **newName, char *extension, char *mode) {


	int newlen;
	char *extaddr;
	int rc;

	// length of input file plus extra space in case "." is missing

	newlen = strlen(oldFileName) + NAMEEXTRA;
	*newName = (char*)malloc(newlen);
	strcpy_s(*newName, newlen, extension);

	if ((extaddr = strstr(*newName, ".")) != NULL) { // Found "." before the end of the extension
		*extaddr = 48; // ASCII for 0, NUL not working for me right now

		strcat_s(*newName, newlen, extension); // Concatenate the extension to the new name

		rc = fopen_s(newfptr, *newName, mode);

		return rc;
	}
}
void common_diagnostics(char *msg) {
	fprintf_s(lisfile, msg);
	printf(msg);

}

unsigned int memory[65535]; // too big?

int main(int argc, char *argv[]) {
	/* Variable Declaration */
	FILE *infile; // the inut asm file pointer
	char *xmename; // pointer to the xme filename
	char *lisname; // pointer to the lis filename
	char *hdrname; //pointer to the filename without path
	char *xmeext; //location of "." before extension in argv[1]
	int xmelen; // length of the XME filename

	if (argc < 2) {
		printf("insufficient arguments\n");
		getchar();
		return 0;
	}

	// fopen_s(): takes infile, checks for NULL, returns 0 if able to open.
	if (fopen_s(&infile, argv[1], "r") != 0) {
		printf("Error opening %s \n ", argv[1]);
		getchar();
		return 1;
	}

	printf("Path of the file dragged in: %s\n", argv[1]);

	char sRecord[200];
	fgets(record, 200, infile);


	if (newFileFromOld(&lisfile, argv[1], &lisname, (char*) ".lis", (char*) "w") != 0) {
		printf("can't open the .lis file \n");
		getchar();
		return 1;
	}
	getchar();

	fclose(infile);
}