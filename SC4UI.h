/*
 SC4UI.h
 
 Edgardo Gutierrez Jr. & Sonia Sarai
 10/28/15
 
 Simulatored a FSM control for a CPU. This version for the LC-2200
 */

/* States for the FSM */
#define FETCH 0
#define DECODE 1
#define EXECUTE 2

/* MicroSates for the Fetch MacroState */
#define iFETCH1 0
#define iFETCH2 1
#define iFETCH3 2
#define iFETCH4 3

/* Microstates for Execute */

#define LDI 0 // REG-IMM
#define LD 1 // REG-BASE-IMM
#define ST 2 // REG-BASE-IMM
//#define PUSH 3 // REG
//#define POP 4 // REG
#define ADD 5 // REG-REG->REG
#define SUB 6 // REG-REG->REG
#define AND 7 // REG-REG->REG
#define OR 8 // REG-REG->REG
#define NOT 9 // REG-REG
//#define SHL 10 // REG-REG->REG
//#define SHR 11 // REG-REG->REG
#define BR 12 // IMM
#define BRZ 13 // IMM
#define BRN 14 // IMM
//#define BRC 15 // IMM
//#define BRO 16 // IMM
//#define JSR 17 // REG
//#define RET 18 // 
//#define RETI 19
//#define TRAP 20
#define HALT 21 

char runFSM(unsigned int *Mem, unsigned int *RegFile, int PC, int A, int B, unsigned int IR, int SW, int mar, int mdr, 
    int state, int microstate, int filesize, int Rx, int Ry, int Rz, int Im);
int debugMonitor(unsigned int *Mem, unsigned int *REGFILE, int PC, int A, int B, unsigned int IR, int SW, int mar, int mdr, int state, int microstate, int filesize);
int loadMemory(unsigned int *Mem, char *fileName, int totalLines);
int convertHexCharToDec(char ch);
unsigned int signExtension(unsigned int theImmediate, int size);
int checkNegative(unsigned int theValue);
void promptLoad(char *fileName);