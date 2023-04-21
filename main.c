#include <stdio.h>
#define ZERO   0b1000
#define BORROW 0b0100
#define CARRY  0b0010
#define PARITY 0b0001

int pc = 0x000;
int sp = 0xFFF;
char a, b;
char opcode;
char flags;
char memory[4096];
int address = 0x0;
char temp = 0;

int limitRegs() {
	// Regs
	a  &= 0xF;
	b  &= 0xF;
	pc &= 0xFFF;
	sp &= 0xFFF;
	// Flags
	flags &= 0xF;
}

int main() {
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
			a += b;
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
					pc += 5;
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
					memory[address]   = pc&0xF00;
					memory[address+1] = pc&0x0F0;
					memory[address+2] = pc&0x00F;
					break;
				case 0b0001: // SP
					memory[address]   = sp&0xF00;
					memory[address+1] = sp&0x0F0;
					memory[address+2] = sp&0x00F;
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
			temp = a;
			a = b;
			b = temp;
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
					memory[sp]   = pc & 0xF00;
					memory[sp+1] = pc & 0x0F0;
					memory[sp+2] = pc & 0x00F;
					sp += 3;
					break;
				case 0b0001: // SP
					memory[sp]   = sp & 0xF00;
					memory[sp+1] = sp & 0x0F0;
					memory[sp+2] = sp & 0x00F;
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
					break;
				case 0b0100: // B
					b = memory[sp];
					sp -= 1;
					break;
				case 0b0010: // PC
					pc = (memory[sp]<<8) | (memory[sp+1]<<4) | (memory[sp+2]);
					sp -= 3;
					break;
				case 0b0001: // SP
					sp = (memory[sp]<<8) | (memory[sp+1]<<4) | (memory[sp+2]);
					// TODO: Figure this behaviour out
					//sp -= 3;
					break;
				default:
					printf("Illegal Register");
					break;
			}
			// TODO: Don't increase if PC was changed
			pc += 2;
			break;
		case 0xC: // JSR f,a
			if (memory[pc+1] & flags) {
				// Increase PC by 5 to return to address following instruction,
				// otherwise it could result in an infinite loop
				// Push PC to Stack
				memory[sp]   = (pc+5) & 0xF00;
				memory[sp+1] = (pc+5) & 0x0F0;
				memory[sp+2] = (pc+5) & 0x00F;
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
			break;
		default:
			printf("Illegal Opcode");
			break;
	}
	
	// Limit all registers to their intended ranges
	limitRegs();
}