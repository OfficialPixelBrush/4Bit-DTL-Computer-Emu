# ZERO   0b1000
# BORROW 0b0100
# CARRY  0b0010
# PARITY 0b0001

int limitRegs() {
	// Regs
	a  &= 0xF;
	b  &= 0xF;
	pc &= 0xFFF;
	sp &= 0xFFF;
	// Flags
	flags &= 0xF;
}

// TODO: Replace individual Flags with flags reg;
int main() {
	int pc = 0x000;
	int sp = 0xFFF;
	char a, b;
	char opcode;
	char flags;
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
				flags &= !CARRY;
			}
			pc += 1;
			break;
		case 4: // SL
			a <<= 1;
			if (a & 0b10000) {
				carry = 1;
			} else {
				carry = 0;
			}
			pc += 1;
			break;
		case 5: // SR
			carry = a & 1;
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
		case 7: 
		default:
			printf("Illegal Opcode");
			break;
	}
}