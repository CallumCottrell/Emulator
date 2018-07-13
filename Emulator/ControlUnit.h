#pragma once


//The Execution prototypes
void execMem_Access();
void execReg_Init();
void execBranch();
void execTwo_Ops();
void execOne_Reg();
void execMem_Access_Relative();

extern unsigned short ir; //Instruction Register
extern unsigned short mbr; // Memory Buffer Register
extern unsigned short mar; // Memory Address Register
extern short int temp; // A temporary Register