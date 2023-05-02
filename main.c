#include <stdio.h>
#include<windows.h>

#define ZERO   0b1000
#define BORROW 0b0100
#define CARRY  0b0010
#define PARITY 0b0001

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

#define BYTE_TO_FLAGS(byte)  \
  ((byte) & 0x08 ? 'Z' : '-'), \
  ((byte) & 0x04 ? 'B' : '-'), \
  ((byte) & 0x02 ? 'C' : '-'), \
  ((byte) & 0x01 ? 'P' : '-') 
int  pc = 0x000;
int  sp = 0xFFF;
char a, b = 0x0;
char opcode = 0x0;
char flags = 0x0;
char memory[4096];
int  address = 0x0;
char temp = 0;

// 1 if Odd Parity  (eg 1,2,4,7,8,11)
// 0 if Even Parity (eg 0,3,5,6,9,10)
int checkParity(char input) {
	int i;
	int parity;
	for (i = 0; i < 8; i++) {
		parity += (input & 1);
		input = input >> 1;
	}
	return parity & 1;
}

// Limit all Registers to their appropriate limits
int limitRegs() {
	// Regs
	a  &= 0xF;
	b  &= 0xF;
	pc &= 0xFFF;
	sp &= 0xFFF;
	
	// Update Zero and Parity Flags here
	// Zero Flag Setting
	if (a == 0) {
		flags |= ZERO;
	} else {
		flags &= ~ZERO;
	}
	
	// Parity Flag Setting
	if (checkParity(a)) {
		flags |= PARITY;
	} else {
		flags &= ~PARITY;
	}
	
	// Flags
	flags &= 0xF;
}

int printStatus() {
	printf("mem[%X]:%X | A:%X B:%X | SP:%X | "BYTE_TO_BINARY_PATTERN"\n", pc, memory[pc], a, b, sp, BYTE_TO_FLAGS(flags));
	return 0;
}

int main() {
	// Initalization
	limitRegs();
	
	// Load file
	FILE *in_file  = fopen("C:/Users/letsp/Desktop/GitHub/4Bit-DTL-Computer/Example/count.bin", "rb"); // read only
	char c;
	int i = 0;
    c = fgetc(in_file);
	while(c != EOF) {
		memory[i]   = (c & 0xF0) >> 4;
		memory[i+1] = (c & 0x0F);
		// printf("%X %X\n", memory[i], memory[i+1]);
		i += 2;
        c = fgetc(in_file);
	}
	fclose(in_file);
	
	// Run the emulator
	while(1) {
		Sleep(1);
		opcode = memory[pc];
		switch(opcode & 0xF) {
			case 0: // NAND
				a = ~(a & b);
				pc += 1;
				break;
			case 1: // NOR
				a = ~(a | b);
				pc += 1;
				break;
			case 2: // NOT
				a = ~a;
				pc += 1;
				break;
			case 3: // ADD
				a = a + b + ((flags & CARRY) >> 1);
				if (a & 0b10000) {
					flags |= CARRY;
				} else {
					flags &= ~CARRY;
				}
				pc += 1;
				break;
			case 4: // SL
				a <<= 1;
				if (a & 0b10000) {
					flags |= CARRY;
				} else {
					flags &= ~CARRY;
				}
				pc += 1;
				break;
			case 5: // SR
				if (a & 0x1) {
					flags |= BORROW;
				} else {
					flags &= ~BORROW;
				}
				a >>= 1;
				pc += 1;
				break;
			case 6: // LD r,n
				switch(memory[pc+1]) {
					case 0b1000: // A
						a = memory[pc+2];
						pc += 3;
						break;
					case 0b0100: // B
						b = memory[pc+2];
						pc += 3;
						break;
					case 0b0010: // PC; opcode reg (loc3 loc2 loc1)
						pc = (memory[pc+2]<<8) | (memory[pc+3]<<4) | (memory[pc+4]);
						break;
					case 0b0001: // SP
						sp = (memory[pc+2]<<8) | (memory[pc+3]<<4) | (memory[pc+4]);
						pc += 5;
						break;
					default:
						printf("Illegal Register");
						break;
				}
				break;
			case 7: // ST r,a
				address = (memory[pc+2]<<8) | (memory[pc+3]<<4) | (memory[pc+4]);
				switch(memory[pc+1]) {
					case 0b1000: // A
						memory[address] = a;
						break;
					case 0b0100: // B
						memory[address] = b;
						break;
					case 0b0010: // PC
						memory[address]   = (pc&0xF00) >> 8;
						memory[address+1] = (pc&0x0F0) >> 4;
						memory[address+2] = (pc&0x00F);
						break;
					case 0b0001: // SP
						memory[address]   = (sp&0xF00) >> 8;
						memory[address+1] = (sp&0x0F0) >> 4;
						memory[address+2] = (sp&0x00F);
						break;
					default:
						printf("Illegal Register");
						break;
				}
				pc += 5; // Same size for all
				break;
			case 8: // LDD r,a | opcode reg addr2 addr1 addr0
				address = (memory[pc+2]<<8) | (memory[pc+3]<<4) | (memory[pc+4]);
				switch(memory[pc+1]) {
					case 0b1000: // A
						a = memory[address];
						break;
					case 0b0100: // B
						b = memory[address];
						break;
					case 0b0010: // PC
						pc = (memory[address]<<8) | (memory[address+1]<<4) | (memory[address+2]);
						break;
					case 0b0001: // SP
						sp = (memory[address]<<8) | (memory[address+1]<<4) | (memory[address+2]);
						break;
					default:
						printf("Illegal Register");
						break;
				}
				pc += 5; // Same size for all
				break;
			case 9: // SWP
				// Shoutout to Programming Memes
				// https://twitter.com/PR0GRAMMERHUM0R/status/1650454565890686976?s=20
				a ^= b;
				b ^= a;
				a ^= b;
				pc += 1;
				break;
			case 0xA: // PSH r
				switch(memory[pc+1]) {
					case 0b1000: // A
						memory[sp] = a;
						sp += 1;
						break;
					case 0b0100: // B
						memory[sp] = b;
						sp += 1;
						break;
					case 0b0010: // PC
						memory[sp]   = (pc & 0xF00) >> 8;
						memory[sp+1] = (pc & 0x0F0) >> 4;
						memory[sp+2] = (pc & 0x00F);
						sp += 3;
						break;
					case 0b0001: // SP
						memory[sp]   = (sp & 0xF00) >> 8;
						memory[sp+1] = (sp & 0x0F0) >> 4;
						memory[sp+2] = (sp & 0x00F);
						// TODO: Figure this behaviour out
						//sp += 3;
						break;
					default:
						printf("Illegal Register");
						break;
				}
				pc += 2;
				break;
			case 0xB: // POP r
				switch(memory[pc+1]) {
					case 0b1000: // A
						a = memory[sp];
						sp -= 1;
						pc += 2;
						break;
					case 0b0100: // B
						b = memory[sp];
						sp -= 1;
						pc += 2;
						break;
					case 0b0010: // PC
						pc = (memory[sp]<<8) | (memory[sp+1]<<4) | (memory[sp+2]);
						sp -= 3;
						break;
					case 0b0001: // SP
						sp = (memory[sp]<<8) | (memory[sp+1]<<4) | (memory[sp+2]);
						pc += 2;
						break;
					default:
						printf("Illegal Register");
						break;
				}
				// TODO: Don't increase if PC was changed
				break;
			case 0xC: // JSR f,a
				if (memory[pc+1] & flags) {
					// Increase PC by 5 to return to address following instruction,
					// otherwise it could result in an infinite loop
					// Push PC to Stack
					memory[sp]   = ((pc+5) & 0xF00) >> 8;
					memory[sp+1] = ((pc+5) & 0x0F0) >> 4;
					memory[sp+2] = ((pc+5) & 0x00F);
					sp += 3;
					// Jump to Address
					pc = (memory[pc+2]<<8) | (memory[pc+3]<<4) | (memory[pc+4]);
				} else {
					// Otherwise just move on
					pc += 5;
				}
				break;
			case 0xD: // RET
				pc = (memory[sp] << 8) | (memory[sp+1] << 4) | memory[sp+2];
				sp -= 3;
				break;
			case 0xE: // JMP f,a
				if (memory[pc+1] & flags) {
					// Jump to Address
					pc = (memory[pc+2]<<8) | (memory[pc+3]<<4) | (memory[pc+4]);
				} else {
					// Otherwise just move on
					pc += 5;
				}
				break;
			case 0xF: // CLR f
				if (memory[pc+1] & (CARRY | BORROW)) {
					flags &= ~memory[pc+1];
				}
				pc += 2;
				break;
			default:
				// This should never occur unless somehow someone adds another number of hexadecimal
				printf("Illegal Opcode");
				break;
		}
		
		// Limit all registers to their intended ranges
		limitRegs();
		printStatus();
	}
}