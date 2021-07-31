#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

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
    Ops ADC, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI, CLV, CMP, CPX, CPY, JMP, JSR, LSR, PHA, PHP, PLA, PLP, ROL, ROR, RTI, RTS, SBC, SEC, SED, SEI
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

// TODO: CLI options to disable printing of ops and memory, set printing start and rows, enable printing address (FFF9?)
int main(int argc, char** argv) {
    // Handle options and file
    bool mem_print = true;
    bool ins_print = true;

    char* file = NULL;

    int start = 0x0000;
    int rows = 8;

    for (int i = 1; i < argc; i++) {
        // Check for options
        if (argv[i][0] == '-') {
            if (argv[i] == string("-?")) {
                printf(
                    "6502 HELP\n"
                    "  Usage:\n"
                    "    6502.exe [-options | --flags] [-f] {file}\n"
                    "\n"
                    "  Options:\n"
                    "    -?        Display help and stop execution (this message).\n"
                    "    -mr {x}   Select the number of rows to print out when displaying memory (default 8).\n"
                    "    -ms {x}   Select the starting value of memory to print rows from. Prefix with \"0x\" for hex input (default 0).\n"
                    "    --m       Disable the printing of memory once hitting a break (default true).\n"
                    "    --i       Disable the printing of instructions while executing (default true).\n"
                    "    -f {x}    Explicitly select the input file to be executed and ignore rest of input (default last arg).\n"
                    );
                return -1;
            }

            else if (argv[i] == string("--m")) {
                mem_print = false;
            }

            else if (argv[i] == string("--i")) {
                ins_print = false;
            }

            else if (argv[i] == string("-mr")) {
                if (argc == i + 1) {
                    printf("-mr requires an argument.\n");
                    return 1;
                }

                // Parse int
                int x = 0;
                stringstream val(argv[i + 1]);

                val >> x;

                rows = x;

                i++;
            }

            // FIXME: Use a more consistent parsing method
            else if (argv[i] == string("-ms")) {
                if (argc == i + 1) {
                    printf("-ms requires an argument.\n");
                    return 1;
                }

                int x = 0;

                stringstream val(argv[i + 1]);

                // Parse hex input
                if (string(argv[i + 1]).substr(0, 2) == "0x") {
                    x = stoul(argv[i + 1], nullptr, 16);
                }

                // Parse int
                else {
                    val >> x;
                }
                
                start = x;

                i++;
            }

            else if (argv[i] == string("-f")) {
                if (argc == i + 1) {
                    printf("-f requires an argument.\n");
                    return 1;
                }

                file = argv[i + 1];
                break;
            }

            else {                    
                // Didn't find option
                printf("Ignoring unknown option: \"%s\"\n", argv[i]);
            }
        }

        else {
            file = argv[i];
            break;
        }
    }

    if (file == NULL) {
        printf("You must input a file. Input \"-?\" for help.\n");
        return 1;
    }

    // Get raw data from file
    ifstream CodeFile(file);

    if (!CodeFile.good()) {
        printf("File \"%s\" not found.\n", file);
        return 1;
    }

    stringstream buffer;
    buffer << CodeFile.rdbuf();

    string codestring = buffer.str();
    
    // Initialize with 0 (not sure why this isn't already)
    for (int i = 0; i < 0x10000; i++) {
        memory[i] = 0;
    }

    // Load code into memory
    for (int i = 0; i < codestring.length(); i++) {
        memory[i] = codestring[i];
    }

    // Set program counter (reset vector)
    pc = 0x100 * memory[0xFFFD] + memory[0xFFFC];

    // Amount of bytes to move (0xFF means break)
    unsigned char mvbytes = 0;

    // Instruction loop
    while (mvbytes != BRK_MOVE) {
        mvbytes = instruction(memory[pc], new unsigned char[] {memory[pc + 1], memory[pc + 2]});

        if (ins_print) printf("%04X %02X - A: %02X X: %02X Y: %02X\n", pc, memory[pc], a, x, y);

        // Increment program counter
        pc += mvbytes;
    }

    if (mem_print) {
        printf("\n");

        // Output of memory once finished (rows of 8 bytes)
        for (int i = 0; i < rows && 0x08 * (1 + i) + start <= 0x10000; i++) {
            for (int j = 0; j < 0x08; j++) {
                printf("%02X ", memory[0x08 * i + j + start]);
            }

            printf("\n");
        }
    }

    return 0;
}