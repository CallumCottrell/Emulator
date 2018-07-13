#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "CPU.h"
#include "Loaders.h"

/* The CPU controls the computer. It goes through the instruction, decode and execute
functions in order to emulate the X Makina. 

Callum Cottrell
B00712510
*/

/* Registers */
//To access a register in the Register file its name is called in the array
short int reg_const[2][NUM_OF_REGISTERS] = { { 0, 0, 0, 0, 0, 0, 0, 0 }, {0 ,1, 2, 4, 8, 0x00FF, 0xFF00, -1} };
unsigned short mar; // Memory Address Register
unsigned short mbr; // Memory Buffer Register 
unsigned short ir; // Instruction Register
unsigned short effective_address; // Effective Address
short int temp; // The temporary Register

// System Clock
unsigned int systemClock = 0;
unsigned int decrementClock = 0;
// Memory (byte or word) 64 kB
union mem memory;

/* The Instruction Cycle prototypes */
void cpu();
void decode();
void fetch();


/*Devices Variables*/
extern struct device dev_array[8];
extern int breakpoint;
extern int interruptFlagSet;
int inDevice;
int inTime;
char inData;
extern FILE *inDevFile;
extern FILE *outDevFile;

// Array for the devices
struct device dev_array[NUMDEV];
void checkDevices();

/* This is the part of the emulation that represents the whole instruction cycle. This
segment of the code runs until a breakpoint is hit or until the user enters ^C. The fetch, decode
and execute functions (execute being within the decode function) are called in order to
properly manipulate the registers and the memory in the way the code intends.
*/
void cpu() {
	int count = 0;
	while (!interruptFlagSet && (breakpoint != reg_const[REGISTER][PC])) {
		
		count++;//debuggingingignig
		decrementClock = 0;
		fetch();
		reg_const[REGISTER][PC] += reg_const[CONSTANT][TWO];
		//Decode the Instruction FRom Fetch I enter Decode. From Decode I will enter Execute...
		decode();
		printf("The program counter: %04x\n", reg_const[REGISTER][PC]);
		getchar();
	}
	//Only try to close the file if it was opened
	if (inDevFile) {
		fclose(inDevFile);
		fclose(outDevFile);
	}
	}

/* The fetch instruction retrieves information from the Memory in order to determine the next
task. It uses the bus function to retrieve the data from the memory at the program counter's value.
It stores the data from the Memory Buffer Register into the Instruction Register.
*/
void fetch() {
	effective_address = reg_const[REGISTER][PC];
	mar = effective_address;

	//If the effctive address is odd then its an interrupt
	if (mar % 2 != 0) {
		printf("This is no good, not fetching on an even boundary\n");
	}

	// Retrieve the next instruction from the memory
	bus(mar, &mbr, READ, WORD);
	//The instruction register equals the Memory Buffer Register
	ir = mbr;
	printf("Instruction Register: %04x\n", ir);
}


/* This function emulates the bus between the CPU and the Memory. Between the are four lines for
transmitting data. The first is the Memory Address Register which contains the place in the memory that
will be accessed. The second, the Memory Buffer Register, is unlike the other lines in that it can return
information to the CPU. The information is either what was in the memory at the specific position or
the new information to the written to the memory. The last two determine if the memory retried or stored is
going to be a byte or a word.*/
void bus(unsigned short mar, unsigned short *mbr, enum ACTION readOrWrite, enum SIZE byteOrWord) {

	//When the bus is accessed it costs 3 clock ticks
	systemClock += 3;
	decrementClock += 3;
	/*Handle Devices - If the memory being accessed is less than 16, it's the devices*/
	if (mar <= 0x0010) {
		// Find the devNum
		int inDev = mar / 2;
		//If its an odd number then its the 
		if (mar % 2 != 0) {
			if (readOrWrite == READ) {
				if (dev_array[inDev].io == INPUT) {
					//Set the DBA bit to 0 in memory and struct. This means input device was successfully read
					dev_array[inDev].dba = 0;
					memory.bytes[mar] &= ~DBA_BIT;
				}
				//store value of the register to the memory buffer register (input or output device)
				*mbr = memory.bytes[mar];
			}
			//If writing
			else {
				//If an output device
				if (dev_array[inDev].io == OUTPUT) {
				
					//Writing to an output device 
					dev_array[inDev].dba = 0;
					//How does the DBA bit get disabled here?
					//memory.bytes[mar] &= ~DBA_BIT;
					memory.bytes[mar] = *mbr;
					//Officially told the device to output to the file. 
					dev_array[inDev].time_left = dev_array[inDev].proc_time + systemClock;
					//The data is now waiting to output and is going to be checked if its time later
					dev_array[inDev].data = *mbr;
					//The device is now trying to transmit and is pending.
					dev_array[inDev].pending = TRUE;
				}
			}
			//Do not continue with the normal memory access if the address is odd.
			return;
		} //Find a way to only access this part if not ODD 
		
	}// If the mar is less than 
			if (readOrWrite == READ) {
			//Make the Memory Buffer Register equal to the byte at the memory and shift the next byte 8 bits to the left to make it the MSByte.
			//MBR= LSB + MSB
			*mbr = (byteOrWord == WORD) ? memory.words[mar >> 1] : memory.bytes[mar];
		}
		//If Writing
		else {
			if (byteOrWord == WORD) {
				memory.words[mar >> 1] = *mbr;
				// find a way to put a 16 bit word into two 8 byte memory addresses
			}
			//If Writing bytes:
			else {
				memory.bytes[mar] = (unsigned char)*mbr;
			}
		}
	}

/* Decode has two phases. The first phase is to find the initial group an instruction belongs to.
The second phase determines more specifically which kind of instruction it is.
Once an instruction has been completely determined, its corresponding execute function is
called. There could be a function called execute(); that does this final step, but
it reduces the need for that class here. A function pointer could also alternatively returned and
called. */

void decode() {
	temp = INST_GROUP(ir);
	unsigned char opCode;

	switch (temp) {
		// 0b11 This case is LDR and STR,
	case 3:
		execMem_Access_Relative();
		break;
		// 0b10 This case is Type 1 or 2
	case 2:
		opCode = OPCODE_5(ir);
		if (opCode > 0b10001) {
			execReg_Init();
		}
		else {
			execMem_Access();
		}
		break;
		//0b01 This case is the Is two operands, reg exchange, or single register
	case 1:
		opCode = OPCODE_8(ir);
		//If the opcode is odd, it is in the bottom four inst of the inst table.  
		if (opCode % 2 != 0) {
			execOne_Reg();
			break;
		}
		else {
			//if it isnt odd then its a two operand instruction
			execTwo_Ops();
		}
		break;

	default:
		execBranch();
		opCode = OPCODE_6(ir);

		break;
	}
	/* End of execution stage */
	/*Check if a device is ready for input.*/
	printf("system clock: %d The inTime: %d ", systemClock, inTime);
	decrementClock++;
	systemClock++;

	//if the user opened a device file then devices are in use.
	if (inDevFile) checkDevices();
}

void checkDevices() {

	while (systemClock >= inTime) {
		//Ensure its an input device, not output
		if (dev_array[inDevice].io == INPUT) {
			if (dev_array[inDevice].dba == 0) {
				dev_array[inDevice].dba = 1;
				memory.bytes[inDevice*2] |= DBA_BIT;
				memory.bytes[(inDevice * 2) + 1] = inData;
				
			}
			//If dba not set then overrun occured
			else {
				dev_array[inDevice].of = 1;
				memory.bytes[inDevice * 2] |= OF_BIT; // SET OVERFLOW
				memory.bytes[inDevice * 2 + 1] = inData;
			}
		}
		if (fscanf_s(inDevFile, "%d %d %c", &inTime, &inDevice, &inData) == EOF) {
			//If its the end of the file, set inTime to be less than current system clock
			//To prevent this while loop from continuing forever (and stop checking input).
			inTime = 0;
			break;
		}
	}
	//struct device *out;
	//If there are any output devices
	/*if (outputDev != -1) {
	out = &dev_array[outputDev];
	while (out) {
	(*out).time_left = (*out).proc_time - decrementClock;
	printf("device output time: %d \n", (*out).proc_time);
	if ((*out).proc_time <= 0) {
	}
	out = out->next;
	}
	}
	*/

	for (int i = 0; i < NUMDEV; i++) {
		/* Three conditions for allowing this device to output
		Must be an output device.
		Enough clock cycles must have passed since attempting to send data.
		Their must be data that is currently waiting to be sent.
		*/
		printf("%d, %d, %d \n", !dev_array[i].io, dev_array[i].time_left, dev_array[i].pending);
		if ((!dev_array[i].io) && (systemClock >= dev_array[i].time_left) && (dev_array[i].pending)) {
			fprintf_s(outDevFile,"Device %d Output: %c \n", i, dev_array[i].data);
			printf("**************Device %d Output: %c \n", i, dev_array[i].data);
			//Device has finished transmitting
			dev_array[i].dba = 1;
			memory.bytes[i * 2] |= DBA_BIT;
			dev_array[i].pending = FALSE;
		}
	}
}