#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Loaders.h"
#include "CPU.h"
#include "ControlUnit.h"

#define GET_MSB_LSB_W(x) ((x) & 0x8001)
#define GET_MSB_LSB_B(x) ((x) & 0x81)

/* This file is responsible for emulating the execution phase of the
X Makina. */
// From the CPU
extern unsigned short int reg_const[2][8];
extern union mem memory;

extern enum SIZE;
extern enum ACTION; 
extern enum INST_TYPE; 
extern enum CONSTANTS;

/* The following Structures and Unions were created
to assist in writing the emulated instructions*/
struct memAccess {
	unsigned int destination : 3;
	unsigned int source : 3;
	unsigned int wb : 1;
	unsigned int other : 1;
	unsigned int format : 3;
	unsigned int type : 3;
	unsigned int category : 2;
};
union memAccessU {
	unsigned short inst;
	struct memAccess Fields;
};
union memAccessU memAccessUnion;

struct memAccessRelative {
	unsigned int destination : 3;
	unsigned int source : 3;
	unsigned int wb : 1;
	unsigned int offset : 6;
	unsigned int type : 1;
	unsigned int category : 2;
};
union memAccessRelativeU {
	unsigned short inst;
	struct memAccessRelative Fields;
};
union memAccessRelativeU memAccessRelativeUnion;

struct regInit {
	unsigned int destination : 3;
	unsigned int bits : 8;
	unsigned int type : 3;
	unsigned int category : 2;
};
union regInitU {
	unsigned short inst;
	struct regInit Fields;
};
union regInitU regInitUnion;

struct branching {
	unsigned int offset : 10;
	unsigned int type : 4;
	unsigned int category : 2;
};
union branchingU {
	unsigned short inst;
	struct branching Fields;
};
union branchingU branchingUFields;

struct twoOps {
	unsigned int destination : 3  ;
	unsigned int SC : 3;
	unsigned int WB : 1;
	unsigned int RC : 1; 
	unsigned int type : 6;
	unsigned int category : 2;
};
union twoOpsU {
	unsigned short inst;
	struct twoOps Fields;
};
union twoOpsU twoOpsUnion;


struct oneRegister {
	unsigned int destination : 3;
	unsigned int spaceA : 3;
	unsigned int wb : 1;
	unsigned int spaceB : 1;
	unsigned int type : 3;
	unsigned int category : 5;
};
union oneRegisterU {
	unsigned short inst;
	struct oneRegister Fields;
};
union oneRegisterU oneRegUFields;

int xor(int A, int B);

//Memory Access execution phase
void execMem_Access() {

	//Set the fields to the instruction register
	memAccessUnion.inst = ir; 
	
	//Making an automatic variables to make the algorithms easier to read.
	unsigned int dest = reg_const[REGISTER][memAccessUnion.Fields.destination]; // Destination register
	unsigned int source = reg_const[REGISTER][memAccessUnion.Fields.source]; // Source register
	unsigned short EA; //The Effective Address to access
	
	// If this is the Load instruction
	if (!memAccessUnion.Fields.type) {
		switch (memAccessUnion.Fields.format) {
			//Pre Increment Register
			case 0b101:
				//  Determine how much to pre increment by.
				source += (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;
			//Pre Decrement Register
			case 0b110:
				//  Determine how much to pre decrement by.
				source -= (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;
			}

		//Set the effective address
		EA = source;
		printf("The source has become: %04x\n", source);
		//Set the memory address register
		mar = EA;
		// Use the bus to access the memory
		bus(mar, &mbr, READ, memAccessUnion.Fields.wb);
		//Set the destination register to the data retrieved from memory
		dest = mbr;
		printf("The destination has become: %04x\n", dest);
		// Pre Decrememnt Register
		switch (memAccessUnion.Fields.format) {
		
			// Post incrment Register
			case 0b001:
				source += (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;

			// Post Decrement Register
			case 0b010:
				source -= (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;
		}
	}
	// Else is a Store instruction
	else {
			//Set the effective Address register
			switch (memAccessUnion.Fields.format) {
				//Pre Increment Register
			case 0b101:
				// EA already contains value of the source register. Determine how much to increment by.
				dest += (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;
				//Pre Decrement Register
			case 0b110:
				dest -= (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;
			}
			EA = dest;
			//Set the memory address register
			mar = EA;
			//Send the data along the bus into memory
			mbr = reg_const[REGISTER][source];
			// Use the bus to access the memory
			bus(mar, &mbr, WRITE, memAccessUnion.Fields.wb);

			// Pre Decrememnt Register
			switch (memAccessUnion.Fields.format) {

				// Post incrment Register
			case 0b001:
				dest += (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;

				// Post Decrement Register
			case 0b010:
				dest -= (memAccessUnion.Fields.wb) ? reg_const[CONSTANT][TWO] : reg_const[CONSTANT][ONE];
				break;
			}
	}

	//Assign the new values of the source and destination registers to the Register File
	//This is ineffecient but makes the code easier on the eyes. Easily switched out if needed
	reg_const[REGISTER][memAccessUnion.Fields.destination] = dest;
	reg_const[REGISTER][memAccessUnion.Fields.source] = source;

}

//Can I reuse the Memory Access function 
void execMem_Access_Relative() {
	
	//Effective Address
	unsigned short EA;

	//Store the instruction register into the fields for ease of use
	memAccessRelativeUnion.inst = ir;

	//Separate variable for the offset manipulation (signed)
	short offset = (short)memAccessRelativeUnion.Fields.offset;

	// If the 6th bit (sign bit) is set, then extend the bit	
	offset |= (GET_SIGN_BIT_RELATIVE(offset)) ?  0xFFC0 : 0;

	//If it is the Store instruction
	if (memAccessRelativeUnion.Fields.type) { 

		// Set the effective address (address of the memory to store to)
		EA = reg_const[REGISTER][memAccessRelativeUnion.Fields.destination] + offset;
		printf("Offset in the store relative function is %04x, and im storing it in... %04x\n", offset, memAccessRelativeUnion.Fields.destination);
		printf("The source data is %c", memAccessRelativeUnion.Fields.source);
		mar = EA;
		// Set the memory buffer register to what will be stored in the memory
		mbr = reg_const[REGISTER][memAccessRelativeUnion.Fields.source];
		// Use the bus to write to the memory;
		bus(mar, &mbr, WRITE, memAccessRelativeUnion.Fields.wb);
		
	}
	//If it is the Load Instruction
	else {
		// Set the effective address (address of the memory to store to)
		EA = reg_const[REGISTER][memAccessRelativeUnion.Fields.source] + offset;

		mar = EA ;
		// Use the bus to write to the memory;
		bus(mar, &mbr, READ, memAccessRelativeUnion.Fields.wb);
		//Store the value retrieved from memory into the destination register
		reg_const[REGISTER][memAccessRelativeUnion.Fields.destination] = mbr;

	}


}
//Register Initialization Instructions Execution Phase
void execReg_Init() {
	//Destination is bits 0-2 regardless of reg_init
	regInitUnion.inst = ir;

	//Making this variable makes the execution line easier to read. I could shoe-horn it in as 
	// reg_const[REGISTER][regInitUnion.regInitFields.destination] but that's a very long variable to read. One Byte never hurt anyone
	char dest = regInitUnion.Fields.destination;
	
	switch (regInitUnion.Fields.type) {
		// MOVL
	case 0b010:
		//Clear the lower part with a mask and bring in the new bits. High byte Unchanged.
		reg_const[REGISTER][dest] = (reg_const[REGISTER][dest] & reg_const[CONSTANT][MASK_LOW]) | regInitUnion.Fields.bits;
		break;

		// MOVLZ 
	case 0b011:
		//Clear the destination register and assign lowbits 
		reg_const[REGISTER][dest] = (reg_const[REGISTER][dest] & reg_const[CONSTANT][ZERO]) | regInitUnion.Fields.bits;
		break;

		// MOVH
	case 0b100:
		//Use a mask to not effect low bytes, bring in the new bits. Low byte unchanged. Shift 
		//Incoming bits to the left 8 to make them MSB.
		reg_const[REGISTER][dest] = (reg_const[REGISTER][dest] & reg_const[CONSTANT][MASK_HIGH]) | (regInitUnion.Fields.bits << 8);
		break;
	}

}

//Branching Instruction Execution phase
void execBranch() {
	printf("The IR is %04x \n",ir);
	/* The amount by which the program counter will be changed (signed) Is stored in Temp Register */
	branchingUFields.inst = ir;
	printf("The IR is %04x \n",ir);
	unsigned char opCode = (char)branchingUFields.Fields.type;
	printf("opCode is %04x", branchingUFields.Fields.type);
	
	//Check if the 4th bit in the type is set. This indicates BL
	if (branchingUFields.Fields.type & 0x08) {

		printf("the IR is now %04x \n", ir);

		temp = branchingUFields.Fields.offset;

		printf("the offset is now %04x \n", temp);

		//Ensures even byte boundary
		temp = temp << 1;
		//Extend the sign bit
		temp = temp | ((temp & 0x0400) ? 0xFC00 : 0);

		printf("the offset is now %04x\n", temp);

		//These operations are reliant on the status of the PSW.
		//Could be sorted by 15..13 = 001 and bits 12-10 increment  decimal 0 -> 8

		switch (branchingUFields.Fields.type) {
			
			//BEQ 000
		case 0b1000:
			// If the Zero flag is set, then the PC is allowed to change.
			reg_const[REGISTER][PC] += (reg_const[REGISTER][PSW] & ZERO_BIT) ? temp : 0; 
			break;

			//BNE/BNZ 001
		case 0b1001:
			reg_const[REGISTER][PC] += !(reg_const[REGISTER][PSW] & ZERO_BIT) ? temp : 0;
			break;

			//BC/BHS 010
		case 0b1010:
			reg_const[REGISTER][PC] += (reg_const[REGISTER][PSW] & CARRY_BIT) ? (unsigned short)temp : 0;
			break;

			//BNC/BLO 011
		case 0b1011:
			reg_const[REGISTER][PC] += !(reg_const[REGISTER][PSW] & CARRY_BIT) ? (unsigned short)temp : 0;
			break;

			//BN 100
		case 0b1100:
			reg_const[REGISTER][PC] += (reg_const[REGISTER][PSW] & NEG_BIT) ? temp : reg_const[CONSTANT][TWO];
			break;

			//BGE 101
		case 0b1101:
			//Neither the Carry bit xnor the Negative bit set in the PSW. The XOR function cleans up the logic.
			// if ~(PSW.N XOR PSW.C) -> change PC
			if (!xor (reg_const[REGISTER][PSW] & NEG_BIT, reg_const[REGISTER][PSW] & CARRY_BIT)) {
				reg_const[REGISTER][PC] += temp;
			}
			break;

			//BLT 110
		case 0b1110:
			//Either the carry bit or the negative bit is set in PSW.
			// if PSW.N xor PSW.C -> change pc
			if (xor (reg_const[REGISTER][PSW] & NEG_BIT, reg_const[REGISTER][PSW] & CARRY_BIT)) {
				reg_const[REGISTER][PC] += temp;
			}
			break;
			//BAL 111
		case 0b1111:
			reg_const[REGISTER][PC] += temp;
			break;
		}
	}

	// If opCode is 0, it's the BL inst:
	else {
		//Set the link register to equal the current Program Counter
		reg_const[REGISTER][LR] = reg_const[REGISTER][PC];
		//Make the offset by masking the IR
		temp = ir & 0x1FFF;
		//Ensure even byte boundary by bit shifting the whole offset to the left one bit 
		temp = SHIFT1BIT(temp);
		//Does the offset have bit 13 set? Then OR the Offset with top bits set high. Otherewise, no sign extending
		temp = temp | ((temp & 0x2000) ? 0xE000 : 0);
		//Add to the program counter
		reg_const[REGISTER][PC] = reg_const[REGISTER][PC] + temp;
	
	}
}

//Two Operand Execution Phase
void execTwo_Ops() {

	//Set the union short
	twoOpsUnion.inst = ir;
	
	/*Easier to make separate variables of int size because arithmetic operations
	in C are done with ints anyway, and the 17th bit can also be checked this way if word
	it is a word operation taking place. Easier to code for Emulation purposes too.
	*/
	int source;
	int destination;
	// A character for checking the old sign against the new sign with
	unsigned char newSign;
	/* A character to hold the sign bit of the operands*/
	char sign;
	//The source is either a constant or a register as determined by the RC bit. The 
	//Retrieves the a decimal equivalent for use in accessing the array. Destination works the 
	//Same way, but is always accessing a register.
	source = reg_const[twoOpsUnion.Fields.RC][twoOpsUnion.Fields.SC];
	destination = reg_const[REGISTER][twoOpsUnion.Fields.destination];

	//If the W/B bit is 0 then a byte is being used. Alter the variables and record sign of the bits
	// In order to determine how the PSW should be set (C or Z)
	if (twoOpsUnion.Fields.WB) {
		destination = (char)(destination); // Typecasting should eliminate the upper parts... ?
		source = (char)(source);
		sign = destination & 0x80;
	}
	else {
		sign = destination & 0x8000;
	}
	//Check if the bits are negative or positive. 

	switch (twoOpsUnion.Fields.type) {

		//ADDC - Let this cascade into ADD since the logic is identical.
	case 0b100010:
		destination = destination + GET_CARRY(reg_const[REGISTER][PSW]); 

		//ADD
	case 0b100000:
		destination = destination + source;
		break;

		//SUBC - destination + not(source) + carry
	case 0b100110:
		destination = destination + ~(GET_CARRY(reg_const[REGISTER][PSW]));
	
		//SUB
	case 0b100100:
		destination = destination + ~(source) + 1;
		break;

		//DADD
	case 0b101000:

		break;

		//CMP
	case 0b0101010:
		// If the destination - source is 0 then PSW.Z = 1, else no change
		reg_const[REGISTER][PSW] |= (!(destination - source) ? ZERO_BIT : 0);
		return;
		break;
		//XOR - made a function
	case 0b101100:
		/*
		0 0 = 0
		1 0 = 1
		0 1 = 1
		1 1 = 0
		*/
		destination = xor(destination, source);
		break;

		//AND
	case 0b101110:
		destination = source & destination;
		break;
		//BIT
	case 0b111000:
		/*Non destructive - ANDS two registers. This is used to "change" the Z and N bits of PSW.
		Assuming change means set to 1, since a result should not be zero and negative at the same
		time*/
		reg_const[REGISTER][PSW] |= ((destination & source) ? (ZERO_BIT + NEG_BIT) : 0);
		break;

		//BIC 
	case 0b110010:
		destination = destination & ~(source);
		break;

		//BIS
	case 0b110100:
		destination = destination | source;
		break;

		//MOV
	case 0b110110:
		destination = source;
		
		//SWAP
		break;
	case 0b110011:
		temp = destination;
		destination = source;
		source = temp;
		break;

	}
	//Lots of Ifs. Can This be cleaned up?
	//If .B
	if (twoOpsUnion.Fields.WB) {
		newSign = destination & 0x80;
		//If the new signs don't match, set the overflow
		if (sign != newSign) {
			reg_const[REGISTER][PSW] |= 0x0010; 
			}
		//If the new sign equals the old sign but the 9th bit (Carry bit) is set
		else if (destination & 0x0100) {
			reg_const[REGISTER][PSW] |= CARRY_BIT; // TO DO: WHAT BIT IS THE CARRY BIT
		}
	}

	else {
		newSign = destination & 0x8000;
		if (sign != newSign) {
			reg_const[REGISTER][PSW] |= 0x0010;; // TO DO: WHAT BIT IS THE OVERFLOW
		}
		//If the new sign equals the old sign but the 17th bit (carry bit) is set
		else if (destination | 0x10000) {
			reg_const[REGISTER][PSW] |= CARRY_BIT; // TO DO: WHAT BIT IS THE CARRY BIT
		}
	}
	//Handle the setting of Negative bit TODO make ternary?
	if (newSign != 0) {
			reg_const[REGISTER][PSW] |= NEG_BIT; // TO DO: WHAT BIS IS NEGATIVE
		}
	else {
			reg_const[REGISTER][PSW] &= ~(NEG_BIT);
	}
	
	if (destination == 0) {
		reg_const[REGISTER][PSW] |= ZERO_BIT;
	}
	else {
		reg_const[REGISTER][PSW] &= ~ZERO_BIT;
	}

	reg_const[REGISTER][twoOpsUnion.Fields.destination] = (unsigned short)destination;

}

//One Register Execution Phase
void execOne_Reg() {
	// Used for setting a carry after the shift
	char setCarry;
	// Set the instruction into the struct for ease of use
	oneRegUFields.inst = ir;
	short dest = oneRegUFields.Fields.destination;

	switch (oneRegUFields.Fields.type) {
	// sra
	case 0b001:
		//If the bottom bit is set, the carry bit will need to be set
		reg_const[REGISTER][PSW] |= (dest % 2 != 0) ? CARRY_BIT : 0;
			//If a word (could be ternary but line becomes too long to read.)
			if (oneRegUFields.Fields.wb) {
				// Save only the MSB and shift everything lower
				dest = (dest & 0x8000) | ((dest & 0xEFFF) >> 1);
			}
			else {
				//Does not save the higher bytes - could add in a way to keep
				dest = (dest & 0x0080) | ((dest & 0x00EF) >> 1);
			}

		break;

	// rrc
	case 0b011:

		//If the bottom bit is set, the carry bit will need to be set
		reg_const[REGISTER][PSW] |= (dest % 2 != 0) ? CARRY_BIT : 0;

			if (oneRegUFields.Fields.wb){
				//Shift the bits (even the sign is shifted)
				dest = dest << 1;
				//if the carry bit is set, set the highest bit of the dest
				dest |= (CARRY_BIT & (reg_const[REGISTER][PSW])) ? 0x8000 : 0;
				}
			//If a byte
			else {
				dest = dest << 1;
				dest |= (CARRY_BIT & (reg_const[REGISTER][PSW])) ? 0x0080 : 0;
			}
			break;

	//swpb
	case 0b101:
		//Store high byte in the temp register
		temp = (dest & 0xFF00) >> 8;
		//shift the low byte to the high byte
		dest = dest << 8;
		//Set the lower byte of the destination to the temp
		dest |= temp;
			break;

	//sxt
	case 0b111:
		dest |= (dest & 0x0080) ? 0xFF00 : 0x0000;
			break;
}

	reg_const[REGISTER][oneRegUFields.Fields.destination] = dest;
}

/* This function can XOR two variables. Comes in handy for BGE, BLT, and XOR*/
int xor(int A, int B) {
	return A = (~A & B) | (A & ~B);
}

