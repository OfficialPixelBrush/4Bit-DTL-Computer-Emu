int main() {
	int pc = 0x000;
	char a, b;
	char opcode;
	char carry;
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
				carry = 1;
			} else {
				carry = 0;
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
					pc = memory[pc+2] memory[pc+3] memory[pc+4]
					break;
				case 0b0001: // SP
					sp = 
					pc += 5;
					break;
				default:
					printf("Illegal Register");
					break;
			}
			break;
		default:
			printf("Illegal Opcode");
			break;
	}
}