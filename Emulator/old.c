#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <signal.h>
#define NAMEEXTRA 8
#define HEX_BIT_3 4096
#define HEX_BIT_2 256
#define HEX_BIT_1 16
/* This file is responsible for taking the S Record as an argument and storing it in to the memory as required*
By Callum Cottrell
B00712510
June 15 2018
*/

/* Global Variables*/
unsigned short lc; // location counter
unsigned short int memory[65536]; // too big?
/*When ctrl c is pressed this function runs*/
void inter() {
	printf("you interrupted me \n");
	//Reinitialize
	signal(SIGINT, (_crt_signal_t)inter);
}

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

/* Convert to integer by subtracting the ASCII numerical value and adding ten if alphabetical.
Eg B = 66 in decimal. Subtract 'A' (i.e. 65) and add 10. Now B = 11 
return -1 if char not applicable to hexadecimal
*/


int load(int argc, char *argv[]) {
	/* Variable Declaration */
	FILE *infile; // the inut asm file pointer
	char *xmename; // pointer to the xme filename
	char *lisname; // pointer to the lis filename
	char *hdrname; //pointer to the filename without path
	char *xmeext; //location of "." before extension in argv[1]
	int xmelen; // length of the XME filename
	char F = 'F';
	char test[] = "S1050100F597ABCDE";
	signal(SIGINT, (_crt_signal_t)inter);
	storeSRecord(test);

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
	/* Read the S-Record*/
	char sRecord[580];
	while (fgets(sRecord, 580, infile)) {

		if (sRecord[0] != 'S') {
			break;
		}
		if (storeSRecord(sRecord) != 1) {
			printf("Checksum error");
			break;
		}
		
	}
	getchar();

	fclose(infile);
}