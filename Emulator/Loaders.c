#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Loaders.h"

/* This file is responsible for reading the file given by the user
and storing it in the memory. Can load S Records or Device input files.

by Callum Cottrell 
B00712510
June 26 2018
*/
#define PC 7 // Program Counter is the seventh Register 

#define MAX_RECORD_LENGTH 78
#define MAX_DEVLINE_LENGTH 16
#define SHIFT3BYTES(x) ((x) << 12)
#define SHIFT2BYTES(x) ((x) << 8)
#define SHIFT1BYTE(x) ((x) << 4)
#define START_OF_DATA 8 // # of Nibbles before Data begins
#define EXTRA_NIBBLES 6 // # of Nibbles in Checksum and address
#define NUMDEV 8

/* External data from the CPU*/
extern unsigned char memory[];
extern unsigned short int reg_const[];

// Array for the devices
struct device dev_array[NUMDEV];
FILE *inDevFile;
FILE *outDevFile;
//Looking for a way to get rid of this function
int length;
int position;
int pairs = 0;
int address = 0;

extern int inDevice;
extern int inTime;
extern char inData;
/*
Take a file containing S Records and decode it into the Memory and find the Initial PC.
Return -1 if error, 1 if success.
*/
int storeSRecord(FILE *sRecords) {

	/*Character buffer for holding the line of the file*/
	char sRecord[MAX_RECORD_LENGTH];

	//Read a line of the file and store it into the character buffer sRecord.
	while (fgets(sRecord, MAX_RECORD_LENGTH, sRecords)) {
		//If the S Record doesn't start with S...
		if (sRecord[0] != 'S') {
			printf("bad news");
			return -1;
		}

		// Calculate the number of byte pairs remaining in the record after 4th char
		sscanf_s(&sRecord[2], "%02x", &pairs);

		// Calculate the address to store the data in for this record		
		sscanf_s(&sRecord[4], "%04x", &address);

		// The Data to store in the array in the form of bytes.
		unsigned int data;
		// Count is the index of the record array that need to be read to.
		//Offset by nibbles to the start of the data (8), exclude the checksum and address nibbles. 
		int nibblesRemaining = (pairs * 2) + (START_OF_DATA - EXTRA_NIBBLES);
		
		//Depending on what is right after the S. S1 or S9 accounted for
		switch (sRecord[1]) {
		case '1':
			//Start at nibble 8, read up to the checksum, and count by bytes at a time.
			for (int i = START_OF_DATA; i < nibblesRemaining; i += 2) {
				sscanf_s(&sRecord[i], "%02x", &data);
				memory[address++] = data;
			}
			break;
		case '9':
			//Set effective address or PC here?
			reg_const[PC] = (unsigned short int)address;
			printf("the program counter will begin at %d \n", address);
			break;

		default:
			sscanf_s(&sRecord[2], "%2x", &length);
			length -= 3;
			printf("The filename is: ");
			for (int i = START_OF_DATA; i < length; i++) {
				printf("%c", sRecord[position+i]);
			}
			break;
		}
	}
	//Return 1 for success
	system("pause");
	fclose(sRecords);
	return 1;
}
/* Initialize the devices*/
void loadDevices(char fileName[MAX_NAME_LENGTH]) {
	//This is so that the file can be accessed by the CPU.c file.
	//This limits the emulator to only having one device file open at a time.
	if (fopen_s(&inDevFile, fileName, "r") != 0) {
		printf("Error opening %s \n", fileName);
	}
	
	int io;
	int proc;
	// The first 8 lines in the file contain the device's processing time and whether its input or output
	for (int i = 0; i < NUMDEV; i++) {
		fscanf_s(inDevFile, "%d %d", &io, &proc);
		dev_array[i].dba = 0;
		dev_array[i].of = 0;
		dev_array[i].data = 0; 
		dev_array[i].io = io;
		dev_array[i].proc_time = proc;
		//If an outputting device
		//Wasteful
		/*if (io == 0) {
			//condition for the first node
			if (list == -1) {
				//Points nowhere
				dev_array[i].next = NULL;
				//Global variable declares where to start the list
				outputDev = i;
				list = i;
			}
			//If not the first node in list
			else {
				//The next node in the list is this output device
				dev_array[list].next = &dev_array[i];
				//Point nowhere
				dev_array[i].next = NULL;
				//Start here next time
				list = i;
			}
		}
		*/
	}
	system("pause");
	// Read the next line in the file to determine first input to device
	// Try not to read the end of the file
	if (!fscanf_s(inDevFile, "%d %d %c", &inTime, &inDevice, &inData)) {
		printf("No device input");
		}

	if (fopen_s(&outDevFile, "Device Output.txt", "w") != 0) {
		printf("Error creating the device output file.");
	}
}