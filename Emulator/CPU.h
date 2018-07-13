#pragma once
/*Constants*/
#define NUM_OF_REGISTERS 8
#define SIZE_OF_MEMORY_BYTES (1<<16)
#define SIZE_OF_MEMORY_WORDS (1<<15)
#define BAD_ADDRESS -1 // An arbitrary constant representing a variable is not in bounds.
#define NOT_EVEN_ADDRESS 11
#define MAX_ADDRESS 63355
#define MIN_ADDRESS 16
#define MAX_NAME_LENGTH 45

/*Constants for accessing the reg_const array*/
#define REGISTER 0
#define CONSTANT 1

/*For accessing the PSW*/
#define CARRY_BIT 0x0001
#define ZERO_BIT 0x0002
#define NEG_BIT 0x0004
#define SLP_BIT 0x0008
#define V_BIT 0x00010

/*Device bits*/
#define DBA_BIT 0x0004
#define IE_BIT 0x0001
#define IO_BIT 0x0002
#define OF_BIT 0x0008

/*Register file constants for accessing by name*/
#define LR 4 // Link Register is the fourth Register
#define SP 5 // Stack Pointer is the fifth Register
#define PSW 6 // Program Status Word is the sixth Register
#define PC 7 // Program Counter is the seventh Register 

/* get rid of a lot of these later*/
#define SHIFT3BYTES(x) ((x) << 12)
#define SHIFT2BYTES(x) ((x) << 8)
#define SHIFT1BYTE(x) ((x) << 4)
#define SHIFT1BIT(x) ((x) << 1)
#define MAKE_BYTE(x) ((x) | 0x00FF)

#define INST_GROUP(x) (((x) & 0xC000) >> 14 ) //This will extract the 2 MSBits

/*Extracting OpCodes*/
#define OPCODE_3(x) (((x) & 0xE000) >> 13 ) // Extract 3 MSB for Opcode
#define OPCODE_5(x) (((x) & 0xF800) >> 11 ) // Extract 5 MSB for OpCode
#define OPCODE_6(x) (((x) & 0xFC00) >> 10 ) // Extract 6 MSB for OpCode
#define OPCODE_8(x) (((x) & 0xFF00) >> 8 ) // Extract 8 MSB for OpCode

/* Extracting information from Operands*/
#define GET_BITS(x) (((x) & 0x07f8) >> 3)
#define GET_DEST(x) ((x) & 0x0007) 
#define GET_OFFSET(x) ((x) & 0x03FF)
#define GET_SIGN_BIT_BRANCHING(x) ((x) & 0x0200)
#define GET_SIGN_BIT_RELATIVE(x) ((x) & 0x0020)
#define GET_SRC_or_CONST(x) ((x) & 0x0080 >> 7) // bit shifting by 7 makes this a 1 or 0
#define GET_SOURCE(x) ((x) &  0x0034)
#define GET_WB_BIT(x) ((x) & 0x0040)
#define GET_CARRY(x) ((x) & 0x0001)
#define GET_SIGN_BYTE(x) ((x) | 0x80)
#define GET_SIGN_WORD(x) ((x) | 0x8000)
#define GET_ZERO(x) ((x) & 0x0010)

/*Enumurations */
enum SIZE { WORD,BYTE }; //For the bus
enum ACTION { READ, WRITE }; // For the bus
enum INST_TYPE { MEM_ACCESS, MEM_ACCESS_RELATIVE, REG_INIT, BRANCHING, TWO_OPS, REG_EXCHANGE, ONE_REG }; // Helps decode
enum CONSTANTS { ZERO, ONE, TWO, FOUR, EIGHT, MASK_HIGH, MASK_LOW, NEG }; // For Accessing the constants

/*Union for accessing the memory of the computer as either a byte or a word*/
union mem {
	unsigned char bytes[SIZE_OF_MEMORY_BYTES];
	unsigned short words[SIZE_OF_MEMORY_WORDS];
};

void cpu();
void bus(unsigned short mar, unsigned short *mbr, enum ACTION readOrWrite, enum SIZE byteOrWord);