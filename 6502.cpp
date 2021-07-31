#include <stdio.h>

// Absolute address
#define ABS_ADDR ops[1] * 0x100 + ops[0]

using namespace std; // XXX: Might cause problems

// Create memory and registers
unsigned char* memory = new unsigned char[64000];

// Accumulator, x and y registers
unsigned char a, x, y;

const unsigned char BRK_MOVE = 0xFF; // Constant that represents a break when returned by an instruction

// Program counter, current place in program
unsigned short pc = 0;

// Function that executes instructions and returns the amount to change pc by
/* TODO:
    Ops ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI, CLV, CMP, CPX, CPY, EOR, JMP, JSR, LSR, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI, RTS, SBC, SEC, SED, SEI
    Addressing Modes (IND, [X, Y]) and (IND), [X, Y]
    Stack Register
*/
unsigned char instruction(unsigned char opcode, unsigned char ops[]) {
    switch (opcode) {
        case 0x00: // BRK (Force Break) Implied
        {
            return BRK_MOVE;
        }

        case 0x81: // STA (IND, X)
        {
            break;
        }

        case 0x84: // STY (Store Index Y in Memory) ZP
        {
            memory[ops[0]] = y;

            return 0x02;
        }

        case 0x85: // STA (Store Accumulator in Memory) ZP
        {
            memory[ops[0]] = a;

            return 0x02;
        }
        
        case 0x86: // STX (Store Index X in Memory) ZP
        {
            memory[ops[0]] = x;

            return 0x02;
        }
        
        case 0x88: // DEY (Decrement Index Y by One) Implied
        {
            y--;

            return 0x01;
        }

        case 0x8A: // TXA (Transfer Index X to Accumulator) Implied
        {
            a = x;

            return 0x01;
        }

        case 0x8C: // STY (Store Index Y in Memory) ABS
        {
            memory[ABS_ADDR] = y;

            return 0x03;
        }

        case 0x8D: // STA (Store Accumulator in Memory) ABS
        {
            memory[ABS_ADDR] = a;

            return 0x03;
        }

        case 0x8E: // STX (Store Index X in Memory) ABS
        {
            memory[ABS_ADDR] = x;

            return 0x03;
        }

        case 0x91: // STA (IND), Y
        {
            break;
        }

        case 0x94: // STY (Store Index Y in Memory) ZP, X
        {
            unsigned char addr = x + ops[0];
            memory[addr] = y;

            return 0x02;
        }

        case 0x95: // STA (Store Accumulator in Memory) ZP, X
        {
            unsigned char addr = x + ops[0];
            memory[addr] = a;

            return 0x02;
        }

        case 0x96: // STX (Store Index X in Memory) ZP, Y
        {
            unsigned char addr = y + ops[0];
            memory[addr] = x;

            return 0x02;
        }

        case 0x98: // TYA (Transfer Index Y to Accumulator) Implied
        {
            a = y;

            return 0x01;
        }

        case 0x99: // STA (Store Accumulator in Memory) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            memory[addr] = a;

            return 0x03;
        }

        case 0x9A: // TXS Implied
        {
            break;
        }

        case 0x9D: // STA (Store Accumulator in Memory) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            memory[addr] = a;

            return 0x03;
        }
        
        case 0xA0: // LDY (Load Index Y with Memory) IMM
        {
            y = memory[ops[0]];

            return 0x02;
        }

        case 0xA1: // LDA (IND, X)
        {
            break;
        }

        case 0xA2: // LDX (Load Index X with Memory) IMM
        {
            x = ops[0];

            return 0x02;
        }

        case 0xA4: // LDY (Load Index Y with Memory) ZP
        {
            y = memory[ops[0]];

            return 0x02;
        }
        
        case 0xA5: // LDA (Load Accumulator with Memory) ZP
        {
            a = memory[ops[0]];

            return 0x02;
        }

        case 0xA6: // LDX (Load Index X with Memory) ZP
        {
            x = memory[ops[0]];

            return 0x02;
        }

        case 0xA8: // TAY (Transfer Accumulator to Index Y) Implied
        {
            y = a;

            return 0x01;
        }

        case 0xA9: // LDA (Load Accumulator with Memory) IMM
        {
            a = ops[0];

            return 0x02;
        }

        case 0xAA: // TAX (Transfer Accumulator to Index X) Implied
        {
            x = a;

            return 0x01;
        }

        case 0xAC: // LDY (Load Index Y with Memory) ABS
        {
            y = memory[ABS_ADDR];

            return 0x03;
        }

        case 0xAD: // LDA (Load Accumulator with Memory) ABS
        {
            a = memory[ABS_ADDR];

            return 0x03;
        }

        case 0xAE: // LDX (Load Index X with Memory) ABS
        {
            x = memory[ABS_ADDR];

            return 0x03;
        }

        case 0xB1: // LDA (IND), Y
        {
            break;
        }

        case 0xB4: // LDY (Load Index Y with Memory) ZP, X
        {
            unsigned char addr = ops[0] + x;
            y = memory[addr];

            return 0x02;
        }

        case 0xB5: // LDA (Load Accumulator with Memory) ZP, X
        {
            unsigned char addr = ops[0] + x;
            a = memory[addr];

            return 0x02;
        }

        case 0xB6: // LDX (Load Index X with Memory) ZP, Y
        {
            unsigned char addr = ops[0] + y;
            x = memory[addr];

            return 0x02;
        }

        case 0xB9: // LDA (Load Accumulator with Memory) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            a = memory[addr];

            return 0x03;
        }

        case 0xBA: // TSX Implied
        {
            break;
        }

        case 0xBC: // LDY (Load Index Y with Memory) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            y = memory[addr];

            return 0x03;
        }

        case 0xBD: // LDA (Load Accumulator with Memory) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            a = memory[addr];

            return 0x03;
        }

        case 0xBE: // LDX (Load Index X with Memory) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            x = memory[addr];

            return 0x03;
        }

        case 0xC6: // DEC (Decrement Memory by One) ZP
        {
            memory[ops[0]]--;

            return 0x02;
        }

        case 0xC8: // INY (Increment Index Y by One) Implied
        {
            y++;

            return 0x01;
        }

        case 0xCA: // DEX (Decrement Index X by One) Implied
        {
            x--;

            return 0x01;
        }

        case 0xCE: // DEC (Decrement Memory by One) ABS
        {
            memory[ABS_ADDR]--;

            return 0x03;
        }

        case 0xD6: // DEC (Decrement Memory by One) ZP, X
        {
            unsigned char addr = ops[0] + x;
            memory[addr]--;

            return 0x02;
        }

        case 0xDE: // DEC (Decrement Memory by One) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            memory[addr]--;

            return 0x03;
        }

        case 0xE6: // INC (Increment Memory by One) ZP
        {
            memory[ops[0]]++;

            return 0x02;
        }

        case 0xE8: // INX (Increment Index X by One) Implied
        {
            x++;

            return 0x01;
        }

        case 0xEA: // NOP (No Operation) Implied
        {
            return 0x01;
        }
        
        case 0xEE: // INC (Increment Memory by One) ABS
        {
            memory[ABS_ADDR]++;

            return 0x03;
        }

        case 0xF6: // INC (Increment Memory by One) ZP, X
        {
            unsigned char addr = ops[0] + x;
            memory[addr]++;

            return 0x02;
        }

        case 0xFE: // INC (Increment Memory by One) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            memory[addr]++;

            return 0x03;
        }
    }

    // Break if not returned

    printf("Broke on opcode %02X\n", opcode);

    return BRK_MOVE;
}

int main(int argc, char* argv[]) {
    // TODO: Load in program from a given binary file
    unsigned char code[64000] = {0xA9, 0xCE, 0xAA, 0xEA, 0x86, 0x20, 0xC8, 0x96, 0x20, 0xC8, 0xE8, 0x96, 0x20, 0xC8, 0x96, 0x20, 0xAA, 0x00};

    // Load code into memory
    for (int i = 0; i < sizeof(code)/sizeof(char); i++) {
        memory[i] = code[i];
    }

    // Amount of bytes to move (0xFF means break)
    unsigned char mvbytes = 0;

    // Instruction loop
    while (mvbytes != BRK_MOVE) {
        mvbytes = instruction(memory[pc], new unsigned char[] {memory[pc + 1], memory[pc + 2]});

        printf("%04d %02X - A: %02X X: %02X Y: %02X\n", pc, memory[pc], a, x, y);

        // Increment program counter
        pc += mvbytes;
    }

    // Output of memory once finished (rows of 8 bytes)
    int rows = 6;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 0x08; j++) {
            printf("%02X ", memory[0x08 * i + j]);
        }

        printf("\n");
    }

    return 0;
}