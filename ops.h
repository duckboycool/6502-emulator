#include <stdio.h>

/* ops.h
  Contains operations and register/memory values.
*/

// Absolute address
#define ABS_ADDR ops[1] * 0x100 + ops[0]

using namespace std; // XXX: Might cause problems

// Create memory and registers
unsigned char* memory = new unsigned char[0x10000];

// Accumulator, x and y registers
unsigned char a, x, y;

// Flags / Status Register
struct StatusRegister {
    bool n = false; // Negative flag
    bool v = false; // Overflow flag
    bool _ = true;  // Unused flag
    bool b = true;  // Break flag (-)
    bool d = false; // Decimal flag
    bool i = true;  // Interrupt disable flag
    bool z = true;  // Zero flag
    bool c = false; // Carry flag

    unsigned char val() {
        unsigned char out = 0;

        out += n * 0b10000000;
        out += v * 0b01000000;
        out += _ * 0b00100000;
        out += b * 0b00010000;
        out += d * 0b00001000;
        out += i * 0b00000100;
        out += z * 0b00000010;
        out += c * 0b00000001;

        return out;
    }
} sr;

const unsigned char BRK_MOVE = 0xFF; // Constant that represents a break when returned by an instruction

// Program counter, current place in program
unsigned short pc;

// Function that executes instructions and returns the amount to change pc by
/* TODO:
    Ops ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CMP, CPX, CPY, JMP, JSR, LSR, PHA, PHP, PLA, PLP, ROL, ROR, RTI, RTS
    Addressing Modes (IND, [X, Y]) and (IND), [X, Y]
*/
unsigned char instruction(unsigned char opcode, unsigned char ops[]) {
    switch (opcode) {
        case 0x00: // BRK (Force Break) Implied
        {
            return BRK_MOVE;
        }

        case 0x01: // ORA (IND, X)
        {
            break;
        }

        case 0x05: // ORA ("OR" Memory with Accumulator) ZP
        {
            a = a | memory[ops[0]];

            return 0x02;
        }

        case 0x09: // ORA ("OR" Memory with Accumulator) IMM
        {
            a = a | ops[0];

            return 0x02;
        }

        case 0x0D: // ORA ("OR" Memory with Accumulator) ABS
        {
            a = a | memory[ABS_ADDR];

            return 0x03;
        }
        
        case 0x11: // ORA (IND), Y
        {
            break;
        }

        case 0x15: // ORA ("OR" Memory with Accumulator) ZP, X
        {
            unsigned char addr = ops[0] + x;
            a = a | memory[addr];

            return 0x02;
        }

        case 0x18: // CLC (Clear Carry Flag) Implied
        {
            sr.c = false;

            return 0x01;
        }

        case 0x19: // ORA ("OR" Memory with Accumulator) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            a = a | memory[addr];

            return 0x03;
        }

        case 0x1D: // ORA ("OR" Memory with Accumulator) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            a = a | memory[addr];

            return 0x03;
        }

        case 0x21: // AND (IND, X)
        {
            break;
        }

        case 0x25: // AND ("AND" Memory with Accumulator) ZP
        {
            a = a & memory[ops[0]];

            return 0x02;
        }

        case 0x29: // AND ("AND" Memory with Accumulator) IMM
        {
            a = a & ops[0];

            return 0x02;
        }

        case 0x2D: // AND ("AND" Memory with Accumulator) ABS
        {
            a = a & memory[ABS_ADDR];

            return 0x03;
        }
        
        case 0x31: // AND (IND), Y
        {
            break;
        }

        case 0x35: // AND ("AND" Memory with Accumulator) ZP, X
        {
            unsigned char addr = ops[0] + x;
            a = a & memory[addr];

            return 0x02;
        }

        case 0x38: // SEC (Set Carry Flag) Implied
        {
            sr.c = true;

            return 0x01;
        }

        case 0x39: // AND ("AND" Memory with Accumulator) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            a = a & memory[addr];

            return 0x03;
        }

        case 0x3D: // AND ("AND" Memory with Accumulator) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            a = a & memory[addr];

            return 0x03;
        }

        case 0x41: // EOR (IND, X)
        {
            break;
        }

        case 0x45: // EOR ("Exclusive-OR" Memory with Accumulator) ZP
        {
            a = a ^ memory[ops[0]];

            return 0x02;
        }

        case 0x49: // EOR ("Exclusive-OR" Memory with Accumulator) IMM
        {
            a = a ^ ops[0];

            return 0x02;
        }

        case 0x4D: // EOR ("Exclusive-OR" Memory with Accumulator) ABS
        {
            a = a ^ memory[ABS_ADDR];

            return 0x03;
        }

        case 0x51: // EOR (IND), Y
        {
            break;
        }

        case 0x55: // EOR ("Exclusive-OR" Memory with Accumulator) ZP, X
        {
            unsigned char addr = ops[0] + x;
            a = a ^ memory[addr];

            return 0x02;
        }

        case 0x58: // CLI (Clear Interrupt Disable Bit) Implied
        {
            sr.i = false;

            return 0x01;
        }

        case 0x59: // EOR ("Exclusive-OR" Memory with Accumulator) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            a = a ^ memory[addr];

            return 0x03;
        }

        case 0x5D: // EOR ("Exclusive-OR" Memory with Accumulator) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            a = a ^ memory[addr];

            return 0x03;
        }

        case 0x61: // ADC (IND, X)
        {
            break;
        }

        case 0x65: // ADC (Add Memory to Accumulator with Carry) ZP
        {
            a = a + memory[ops[0]] + sr.c;

            sr.c = (memory[ops[0]] + sr.c > a);

            return 0x02;
        }

        case 0x69: // ADC (Add Memory to Accumulator with Carry) IMM
        {
            a = a + ops[0] + sr.c;

            sr.c = (ops[0] + sr.c > a);

            return 0x02;
        }

        case 0x6D: // ADC (Add Memory to Accumulator with Carry) ABS
        {
            a = a + memory[ABS_ADDR] + sr.c;

            sr.c = (memory[ABS_ADDR] + sr.c > a);

            return 0x03;
        }

        case 0x71: // ADC (IND), Y
        {
            break;
        }

        case 0x75: // ADC (Add Memory to Accumulator with Carry) ZP, X
        {
            unsigned char addr = ops[0] + x;
            a = a + memory[addr] + sr.c;

            sr.c = (memory[addr] + sr.c > a);

            return 0x02;
        }

        case 0x78: // SEI (Set Interrupt Disable Status) Implied
        {
            sr.i = true;

            return 0x01;
        }

        case 0x79: // ADC (Add Memory to Accumulator with Carry) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            a = a + memory[addr] + sr.c;

            sr.c = (memory[addr] + sr.c > a);

            return 0x03;
        }

        case 0x7D: // ADC (Add Memory to Accumulator with Carry) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            a = a + memory[addr] + sr.c;

            sr.c = (memory[addr] + sr.c > a);

            return 0x03;
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
        
        case 0xB8: // CLV (Clear Overflow Flag)
        {
            sr.v = false;

            return 0x01;
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

        case 0xD8: // CLD (Clear Decimal Mode) Implied
        {
            sr.d = false;

            return 0x01;
        }

        case 0xDE: // DEC (Decrement Memory by One) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            memory[addr]--;

            return 0x03;
        }

        case 0xE1: // SBC (IND, X)
        {
            break;
        }

        case 0xE5: // SBC (Subtract Memory from Accumulator with Borrow) ZP
        {
            a = a - memory[ops[0]] - !sr.c;

            sr.c = (- memory[ops[0]] - !sr.c < a);

            return 0x02;
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

        case 0xE9: // SBC (Subtract Memory from Accumulator with Borrow) IMM
        {
            a = a - ops[0] - !sr.c;

            sr.c = (- ops[0] - !sr.c < a);

            return 0x02;
        }

        case 0xEA: // NOP (No Operation) Implied
        {
            return 0x01;
        }

        case 0xED: // SBC (Subtract Memory from Accumulator with Borrow) ABS
        {
            a = a - memory[ABS_ADDR] - !sr.c;

            sr.c = (- memory[ABS_ADDR] - !sr.c < a);

            return 0x03;
        }
        
        case 0xEE: // INC (Increment Memory by One) ABS
        {
            memory[ABS_ADDR]++;

            return 0x03;
        }

        case 0xF1: // SBC (IND), Y
        {
            break;
        }

        case 0xF5: // SBC (Subtract Memory from Accumulator with Borrow) ZP, X
        {
            unsigned char addr = ops[0] + x;
            a = a - memory[addr] - !sr.c;

            sr.c = (- memory[addr] - !sr.c < a);

            return 0x02;
        }

        case 0xF6: // INC (Increment Memory by One) ZP, X
        {
            unsigned char addr = ops[0] + x;
            memory[addr]++;

            return 0x02;
        }

        case 0xF8: // SED (Set Decimal Mode) Implied
        {
            sr.d = true;

            return 0x01;
        }

        case 0xF9: // SBC (Subtract Memory from Accumulator with Borrow) ABS, Y
        {
            unsigned short addr = ABS_ADDR + y;
            a = a - memory[addr] - !sr.c;

            sr.c = (- memory[addr] - !sr.c < a);

            return 0x03;
        }

        case 0xFD: // SBC (Subtract Memory from Accumulator with Borrow) ABS, X
        {
            unsigned short addr = ABS_ADDR + x;
            a = a - memory[addr] - !sr.c;

            sr.c = (- memory[addr] - !sr.c < a);

            return 0x03;
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
