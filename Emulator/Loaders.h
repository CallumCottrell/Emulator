
/*
This Header file is necessary for the debugger to access the store function
of the loader.
Callum Cottrell
B00712510
*/
#define NUMDEV 8
#define MAX_NAME_LENGTH 35
#define TRUE 1
#define FALSE 0
#define INPUT 1
#define OUTPUT 0
int storeSRecord(FILE *sRecords);
void loadDevices(char fileName[MAX_NAME_LENGTH]);


/* A way of representing a device in the emulation separate from the memory.*/
struct device
{
	unsigned int ie : 1; /*Interrupts enabled?*/
	unsigned int io : 1; /* IO: INPUT or OUTPUT */
	unsigned int dba : 1; /* Input read? Output completed? */
	unsigned int of : 1; /* Overflow detected */
	signed int proc_time; /* Time to processes an output byte */
	signed int time_left; /* Time remaining to finish output */
	unsigned char data; /* Byte read or written */
	unsigned int pending; /* True or False. If device waiting to output...*/
};


