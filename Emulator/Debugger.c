#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Loaders.h"
#include "CPU.h"
#include <signal.h> /* Signal handling software */
/*The main is contained within this file. From this file the user can use the
X Makina emulator.

Callum Cottrell
B00712510
*/

#define TRUE 1
#define FALSE 0

// Local Function protoypes
void registerMenu();
void memoryMenu();

// External Variables
extern unsigned short int reg_const[2][NUM_OF_REGISTERS];
extern union mem memory;

// Local Global Variables
int breakpoint = 0; // Where the instruction cycle will end
int reg; //The register the user wishes to access
int newValue; // The new value the user wishes to assign mem or reg
int uppBound; // The upp bound of the memory to view
int lowBound; // The low bound of the memory to view
int newAddress;
FILE *newFile;

int interruptFlagSet = FALSE;

void sigint_hdlr()
{
	/*
	- Invoked when SIGINT (control-C) is detected
	- changes state of waiting_for_signal
	- signal must be reinitialized
	*/
	interruptFlagSet = TRUE;
	signal(SIGINT, (_crt_signal_t)sigint_hdlr); /* Reinitialize SIGINT */
}
void catch_abort()
{
	printf("hello");
}
int main(int argc, char *argv[]) {

	signal(SIGINT, (_crt_signal_t)sigint_hdlr);
	char fileName[MAX_NAME_LENGTH];
	int newAddress;
	char quit = 0;
	char choice = 0;

	while (quit != 1) {
		printf("=======X-Makina Debugger======= \n");
		printf("Enter a choice from the list. \n");
		printf("B: To add a breakpoint \nR: To view or edit the Register File \nM: To view the Memory \n");
		printf("O: To Open an SRecord \nG: To Start the program \nD: To add a Device \nQ: To Quit the program\n");
		scanf_s("%c", &choice);
		// This isn't very good because it isn't portable. However, Visual studio library already not too portable anyway.
		system("@cls");

		switch (choice) {
		/* Add a breakpoint*/
		case 'b':
		case 'B': printf("Where do you want to add a Breakpoint?");
			do {
				scanf_s(" %x", &breakpoint);
			} while (isInBounds(breakpoint) == BAD_ADDRESS);
			break;

		/* Register menus*/
		case 'r':
		case 'R':
			registerMenu();
			break;

		/* Memory menus*/
		case 'm':
		case 'M':
			memoryMenu();
			break;
		/*Open and Read an S Record*/
		case 'o':
		case 'O':
			//Reusing quit to save space and variable declarations.
				printf("What is the name of the file you want to read \n");
				scanf_s("%s", fileName, MAX_NAME_LENGTH);
				printf("the file youre trying to read is %s \n", fileName);

				if (fopen_s(&newFile, fileName, "r") != 0) {
					printf("Error opening %s, would you like to try again? Y or N\n ", fileName);
					scanf_s(" %c", &choice);
				}	
				else {
					storeSRecord(newFile);

				}
			//Reset quit
			quit = 0;
			break;

			//Add a device
		case 'd':
		case 'D':
			printf("What is the name of the device file you want to read \n");
			scanf_s("%s", fileName, MAX_NAME_LENGTH);
			printf("the file youre trying to read is %s \n", fileName);
			loadDevices(fileName);
			break;

			//Go! Start.
		case 'g':
		case 'G':
			cpu();
			break;

			//Quit
		case 'q':
		case 'Q':
			quit = 1;
			break;

		default: printf("Please enter a valid command. \n");
		}

		system("@cls");
	}
	return 0;
}
/* Function for the memory altering and viewing. This allows the user to declare a lower and upper 
bound for viewing the memory. The user can also request to edit the memory to be any value.
*/
void memoryMenu() {
	char quit = 0;
	char choice = 0;

		printf("======Memory Menu======\n");
		printf("Would you like to change a memory address or view the memory? \n V = View, and C = Change, Q = Quit \n");
		while (quit == 0) {
		scanf_s(" %c", &choice);
		// If the user wants to View the memory
		switch (choice) {
		case 'V':
		case 'v':
			printf("Indicate the lower bound of memory to display.\n");

			do {
				scanf_s(" %04x", &lowBound);
			} while (isInBounds(lowBound) == BAD_ADDRESS);

			printf("Indicate the upper bound of memory to display.\n");

			do {
				scanf_s(" %04x", &uppBound);
				if (lowBound > uppBound) {
					printf("Lowerbound greater than Upperbound, so ");
					uppBound = BAD_ADDRESS;
				}
			} while (isInBounds(uppBound) == BAD_ADDRESS);

			printf("Address    Contents\n");
			for (lowBound; lowBound < uppBound; lowBound++) {
				printf("%04x:           %02x \n", lowBound, memory.bytes[lowBound]);
			}
			break;

		case 'C':
		case 'c':
			printf("What address do you want to change?\n");
			do {
				scanf_s(" %04x", &newAddress);
			} while (isInBounds(newAddress) == BAD_ADDRESS);

			printf("What value do you want the address to be?\n");
			do {
				scanf_s(" %04x", &newValue);
			} while (isValidAddress(newValue) == BAD_ADDRESS);
			memory.bytes[newAddress] = newValue;

			break;
		case 'Q':
		case 'q':
			quit = 1;
			break;
		default:
			printf("Invalid command is :%c \n", choice);
		}
	}
}

void registerMenu() {

	printf("======Register Menu======\n");
	//Loop through the register file and print each register
	for (int i = 0; i < 8; i++) {
		printf("R%d = %04x \n", i, reg_const[REGISTER][i]);
	}
	
	printf("Enter the number of the register you would like to change or -1 to back out. \n");
	
	do {
		scanf_s("%d", &reg);
		if (reg < 0) {
			return;
		}// return to the menu without doing anything
	} while (isValidRegister(reg) == BAD_ADDRESS);

	printf("Enter value you would like to set R%d to. \n", reg);

		do {
			scanf_s("%04x", &newValue);
		} while (isInBounds(newValue) == BAD_ADDRESS);
		reg_const[REGISTER][reg] = newValue;

}

int isInBounds(int address) {
	//Check if the bound fits in the memory array
	if (address > MAX_ADDRESS || address < MIN_ADDRESS) {
		printf("Invalid Address. Try again. \n");
		return BAD_ADDRESS;
	}
	return 1;
}

int isValidRegister(int reg) {
	if (reg > 8 || reg < 0) {
		printf("The Register given is not valid \n");
		return BAD_ADDRESS;
	}
	 return 1;
}

int isValidAddress(int value) {
	if (value > 65535 || value < 0) {
		printf("The value does not fit");
		return BAD_ADDRESS;
	}
	return 1;
}