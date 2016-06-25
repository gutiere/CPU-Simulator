/*
 
 SC4UI.c
 
 Edgardo Gutierrez Jr. & Sonia Sarai

 11/5/15
 
 Simulatored a FSM control for a CPU. This version for the LC-2200
 
 */

#import <stdio.h>
#import <stdlib.h>
#include "SC4UI.h"

int main () {
    // (status word) with bits 31..28 as flags, Z, N, C, O
    int PC = 0, A = 0, B = 0, SW = 0, mar = 0, mdr = 0, Rx = 0, Ry = 0, Rz = 0, Im = 0, state = FETCH, microstate = iFETCH1, filesize = 16, i;
	unsigned int ir = 0, Mem[100],  RegFile[16];
	char UserInput, *fileName = malloc(100 * sizeof(char));
	// Filling with random numbers
	for (i = 0; i < 100; i++) {
        if (i < 16) RegFile[i] = rand(); //NOTE: Registers 0, 1, 2 are RX, RY, RZ respectively 
    	Mem[i] = rand();
    }
    UserInput = debugMonitor(Mem, RegFile, PC, A, B, ir, SW, mar, mdr, state, microstate, filesize);
    while (UserInput != '0') {
    	switch (UserInput) {
    		case '1': 
    			promptLoad(fileName);
    			// loadMemory(MEM, fileName, filesize);
    			while (loadMemory(Mem, fileName, filesize) == 0) {
    				printf("Error loading that file, please try again.\n");
    				promptLoad(fileName);
    			}
  		  		break;	
    	}
    	UserInput = runFSM(Mem, RegFile, PC, A, B, ir, SW, mar, mdr, state, microstate, filesize, Rx, Ry, Rz, Im);
    }
}

char runFSM(unsigned int *Mem, unsigned int *RegFile, int PC, int A, int B, unsigned int ir, int SW, int mar, int mdr, 
    int state, int microstate, int filesize, int Rx, int Ry, int Rz, int Im) {
    	char UserInput;
    	int running = 1;
	    while (running == 1) { // Do until halt executed
		switch (state) {
            case FETCH:
                while (microstate != iFETCH4) {
                    switch (microstate) {
                        case iFETCH1:
                            mar = PC;
                            A = PC;
                            microstate = iFETCH2;
                            break;
                        case iFETCH2:
                            ir = Mem[mar];
                            microstate = iFETCH3;
                            break;
                        case iFETCH3:
                            PC = A + 1;
                            microstate = iFETCH4;
                            break;
                    }
                }
                state = DECODE; // iFETCH4 technically
				break;
            case DECODE:
                microstate = (ir >> 27); // microState <- ir[31..26]
				switch (microstate) { // Based on the opcode
                    case LDI:
                        Rx = (ir >> 23) & 0xF; // RegX <- ir[26..22]
                        // rd <- ext(immed23)
						Im = signExtension((ir & 0x7FFFFF), 23); 
						break;
                    case LD:
                        Rx = (ir >> 23) & 0xF; // RegX <- ir[26..22]
                        Ry = (ir >> 19) & 0xF; // RegY <- ir[22..18]
                        Im = ir & 0x7FFFF; // IMMREG <- ir[18..0]
						break;
					case ST:
                        Rx = (ir >> 23) & 0xF; // RegX <- ir[26..22]
                        Ry = (ir >> 19) & 0xF; // RegY <- ir[22..18]
                        // rd <- ext(immed19)
                        Im = signExtension((ir & 0x7FFFF), 19); 
						break;
                    case ADD: case SUB: case AND:
                        Rx = (ir >> 23) & 0xF; // RegX <- ir[26..22]
                        Ry = RegFile[(ir >> 19) & 0xF]; // RegY <- ir[22..18]
                        Rz = RegFile[ir & 0xF]; // RegZ <- ir[3..0]
						break;
                    case OR:
                        Rx = (ir >> 23) & 0XF; // RegX <- ir[26..22]
                        Ry = RegFile[(ir >> 19) & 0XF]; // RegY <- ir[22..18]
                        Rz = RegFile[ir & 0xF]; // RegZ <- ir[3..0]
						break;
                    case NOT:
                        Rx = (ir >> 23) & 0xF; // RegX <- ir[26..22]
                        Ry = (ir >> 19) & 0xF; // RegY <- ir[22..18]
						break;
                    case BR: case BRZ: case BRN:
						// rd <- ext(immed23)
						Im = signExtension((ir & 0x7FFFFF), 23); 
						break;
                }
                state = EXECUTE;
				break;
            case EXECUTE:
                switch (microstate) {
                    case LDI:
                    	if (checkNegative(Im) == 1) {
                    		Im = Im & 0x7FFFFF;
                    	}
						RegFile[Rx] = Im;
						break;
                    case LD:
                    	if (checkNegative(Im) == 1) {
                    		Im = Im & 0x7FFFFF;
                    		RegFile[Rx] = Mem[Ry - Im];
                    	} else RegFile[Rx] = Mem[Im + Ry];
						break;
                    case ST:
                    	if (checkNegative(Mem[Im]) == 1) {
                    		Mem[Im] = Mem[Im] & 0x7FFFFF;
                    		Mem[Ry - Im] = RegFile[Rx];
                    	} else Mem[Ry + Im] = RegFile[Rx];
                        Mem[Im + Ry] = RegFile[Rx];
						break;
                    case ADD:
                    	SW = ~(~SW | 0xF0000000);
                        A = Ry;
                        B = Rz;
                        RegFile[Rx] = A + B;
                        // Addition of 2 signed #s resulting in an unsigned #.
                        if ((((A & 0x80000000) == 0x80000000) 
                        	&& ((B & 0x80000000) == 0x80000000))
                        	&& ((RegFile[Rx] & 0x80000000) == 0)) {
                        	SW = SW | 0x10000000;
						// Addition of 2 unsigned #s resulting in an signed #.
                        } else if ((((A & 0x80000000) == 0) && ((B & 0x80000000) == 0))
                        	&& ((RegFile[Rx] & 0x80000000) == 0x80000000)) {
                        	SW = SW | 0x10000000;
						// Addition of 1 signed + 1 unsigned # resulting in an unsigned #.
                        } else if ((((A & 0x80000000) == 0) 
                        	&& ((B & 0x80000000) == 0x80000000))
                        	&& ((RegFile[Rx] & 0x80000000) == 0)) {
                        	SW = SW | 0x20000000;
                        // Addition of 1 signed + 1 unsigned # resulting in an unsigned #.
                        } else if ((((B & 0x80000000) == 0) 
                        	&& ((A & 0x80000000) == 0x80000000))
                        	&& ((RegFile[Rx] & 0x80000000) == 0)) {
                        	SW = SW | 0x20000000;
                        } 						
                    	break;
                    case SUB:
                    	SW = ~(~SW | 0xF0000000);
                        A = Ry;
                        B = Rz;
                        RegFile[Rx] = A - B;
                    	// Subtraction of two unsigned #s resulting in an signed #.
                        if ((((A & 0x80000000) == 0) && ((B & 0x80000000) == 0))
                        	&& ((RegFile[Rx] & 0x80000000) == 0x80000000)) {
                        	SW = SW | 0x20000000;
                        }
						break;
                    case AND:
                        A = Ry;
                        B = Rz;
                        RegFile[Rx] = A & B;
						break;
                    case OR:
                        A = Ry;
                        B = Rz;
                        RegFile[Rx] = A | B;
						break;
                    case NOT:
                        RegFile[Rx] = ~Ry;
						break;
                    case BR:
                    	if (checkNegative(Im) == 1) {
                			Im = ~(~Im | 0xFFC00000);
                    		PC = PC - Im;
                    	} else PC = PC + Im;
                    	break;
                    case BRZ:
                        if ((SW & 0x80000000) == 0x80000000) {
                    		if (checkNegative(Im) == 1) {
                				Im = ~(~Im | 0xFFC00000);
                    			PC = PC - Im;
                    		} else PC = PC + Im;
                        }
						break;
					case BRN:
                        if ((SW & 0x40000000) == 0x40000000) {
                    		if (checkNegative(Im) == 1) {
                				Im = ~(~Im | 0xFFC00000);
                    			PC = PC - Im;
                    		} else PC = PC + Im;
                        }
						break;
                    case HALT:
						running = 0;
						switch(state) {
							case FETCH:
								printf("                        [LAST STATE: FETCH]\n");
								break;
							case DECODE:
								printf("                        [LAST STATE: DECODE]\n");
								break;
							case EXECUTE:
								printf("                        [LAST STATE: EXECUTE]\n");
								break;               
						}
                        break;
                }
                UserInput = debugMonitor(Mem, RegFile, PC, A, B, ir, SW, mar, mdr, state, 
                						microstate, filesize);
                state = FETCH;
                microstate = iFETCH1;
        }
    }
    return UserInput;
}

int debugMonitor(unsigned int *Mem, unsigned int *RegFile, int PC, int A, int B, unsigned int ir, int SW, int mar, int mdr, int state, int microstate, int filesize) {
	printf("\n SC-4 Debug Monitor	Brought to you by Edgardo Gutierrez Jr. & Sonia Sarai\n\n");
	printf(" Register File                  Memory Dump\n");
	char cursor;
	int i;
	for (i = 0; i < filesize; i++) {
		if (ir == Mem[i]) {
			cursor = '>';
		} else cursor = ' ';
    	printf("%X: %08X              %c %08X: %08X\n", i, RegFile[i], cursor, i, Mem[i]);
	}
	printf("\nPC: %08X  ir: %08X SW: %08X\n", PC, ir, SW);
	printf("mar: %08X mdr: %08X ALU.A: %08X ALU.B: %08X ALU.R: %08X", mar, mdr, A, B, (A + B));
		printf("\n\nCommands: 1 = Load, 2 = Step, 3 = Run, 4 = Memory     Enter: ");
	 	return getchar();
}

int loadMemory(unsigned int *Mem, char *fileName, int totalLines) {
	int colNumber = 0, hexNumber = 0;
	int bool = 0; 
	if (fopen(fileName, "r") == NULL) return 0;
	FILE *ioFile;
	ioFile = fopen(fileName, "r");	
	char ch = fgetc(ioFile);
	while(ch != EOF) {	
		if (ch != '\n') {
			if (ch == ':') {
				bool = 1;
			} else if (bool == 0) { // bool = 0: col number
				if (colNumber == 0) {
					colNumber = convertHexCharToDec(ch);
				} else {
					colNumber = (colNumber * 16) + convertHexCharToDec(ch);
				}
			} else if (bool == 1) { // bool = 1: hex number
				if (hexNumber == 0) {
					hexNumber = convertHexCharToDec(ch);
				} else {
					hexNumber = (hexNumber * 16) + convertHexCharToDec(ch);
				}
			}
		} else {
			Mem[colNumber] = hexNumber;
			colNumber = 0;
			hexNumber = 0;
			bool = 0;
		}
		ch = fgetc(ioFile);
    }
    fclose(ioFile);
    return 1;
}

int convertHexCharToDec(char ch) {
	switch(ch) {
		case 'A':
			return 10;
		case 'B':
			return 11;
		case 'C':
			return 12;
		case 'D':
			return 13;
		case 'E':
			return 14;
		case 'F':
			return 15;
	}
	return (ch - '0');
}

unsigned int signExtension(unsigned int theImmediate, int size) {
	unsigned int check;
	unsigned int fill;
	if (size == 23) {
		check = 0x400000;
		fill = 0xFF800000;
	} else if (size == 19) {
		check = 0x40000;
		fill = 0xFFF80000;
	} else {
		printf("Invalid size: %d", size);
	}
	if ((theImmediate & check) == check) {
		return fill | theImmediate;
	} else return theImmediate;
}

int checkNegative(unsigned int theValue) {
	if ((0xF0000000 & theValue) == 0xF0000000) {
		return 1;
	} else return 0;
}

void promptLoad(char *fileName) {
	printf("Enter name of file to load: ");
	scanf("%s", fileName);
}