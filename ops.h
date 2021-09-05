#include <stdio.h>

/* ops.h
  Contains operations and register/memory values.
*/

#define byte unsigned char
#define byte2 unsigned short

// Absolute address
#define ABS_ADDR ops[1] * 0x100 + ops[0]

using namespace std; // XXX: Might cause problems

// Create memory and registers
byte* memory = new byte[0x10000];

// Accumulator, x and y registers
byte a, x, y;

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

    byte val() {
        byte out = 0;

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

const byte BRK_MOVE = 0xFF; // Constant that represents a break when returned by an instruction

// Program counter, current place in program
byte2 pc;

// Option flags
// Printout of memory and instructions
bool mem_print = true;
bool asc_print = true;
bool ins_print = true;
// Print value if on printing address and enabled
byte print_addr = 0xFFF9;
bool print_out = false;

string endprint = ""; // Printed out after program stops

void st_print(byte addr, int val) {
    if (addr == print_addr && print_out) {
        // Print at end if printing memory or instructions (so no overlap), otherwise, print now
        if (mem_print || ins_print) {
            endprint += (char)val;
            return;
        }

        printf("%c", (char)val);
    }
}

// Define addressing modes
#define Accum (a);\
    return 0x01;               // Accumulator as an argument
#define IMM (ops[0]);\
    return 0x02;               // Literal first value as an argument
#define ABS (ABS_ADDR);\
    return 0x03;               // Value at address of next two bytes
#define ZP (memory[ops[0]]);\
    return 0x02;               // Value at address of next byte with high byte (page) of 0
#define ZP_X \
  ((byte)(memory[ops[0]] + x));\
    return 0x02;               // Value at address of next byte with high byte 0, indexed to x
#define ZP_Y \
  ((byte)(memory[ops[0]] + y));\
    return 0x02;               // ^, indexed to y
#define ABS_X \
  ((byte2)(ABS_ADDR + x));\
    return 0x03;               // Value at address of next two bytes, indexed to x
#define ABS_Y \
  ((byte2)(ABS_ADDR + y));\
    return 0x03;               // ^, indexed to y
#define Implied ();\
    return 0x01;               // No arguments (implied from instruction)
// #define Relative
// #define IND_X
// #define IND_Y
// #define Indirect

// Define instructions
template <typename T>
void ORA(T val) {
    a = a | val;
}

template <typename T>
void ASL(T&& addr) { // Evil
    sr.c = addr / 0x80;

    addr = addr * 0b10;
}

void CLC() {
    sr.c = false;
}

template <typename T>
void ROL(T&& addr) {
    bool bit = addr / 0x80;

    addr = addr * 0b10 + sr.c;

    sr.c = bit;
}

template <typename T>
void AND(T val) {
    a = a & val;
}

void SEC() {
    sr.c = true;
}

template <typename T>
void EOR(T val) {
    a = a ^ val;
}

template <typename T>
void LSR(T&& addr) {
    sr.c = addr % 0b10;

    addr = addr / 0b10;
}

void CLI() {
    sr.i = false;
}

template <typename T>
void ADC(T val) {
    a = a + val + sr.c;

    sr.c = (val + sr.c > a);
}

template <typename T>
void ROR(T&& addr) {
    bool bit = addr % 0b10;

    addr = 0x80 * sr.c + addr / 0b10;

    sr.c = bit;
}

void SEI() {
    sr.i = true;
}

template <typename T>
void STA(T&& addr) {
    st_print(addr, a);

    addr = a;
}

template <typename T>
void STY(T&& addr) {
    st_print(addr, y);

    addr = y;
}

template <typename T>
void STX(T&& addr) {
    st_print(addr, x);

    addr = x;
}

void DEY() {
    y--;
}

void TXA() {
    a = x;
}

void TYA() {
    a = y;
}

template <typename T>
void LDY(T val) {
    y = val;
}

template <typename T>
void LDA(T val) {
    a = val;
}

template <typename T>
void LDX(T val) {
    x = val;
}

void TAY() {
    y = a;
}

void TAX() {
    x = a;
}

void CLV() {
    sr.v = false;
}

template <typename T>
void DEC(T&& addr) {
    addr--;
}

void INY() {
    y++;
}

void DEX() {
    x--;
}

void CLD() {
    sr.d = false;
}

template <typename T>
void SBC(T val) {
    a = a - val - !sr.c;

    sr.c = (- val - !sr.c < a);
}

template <typename T>
void INC(T&& addr) {
    addr++;
}

void INX() {
    x++;
}

void NOP() {
    
}

void SED() {
    sr.d = true;
}

// Function that executes instructions and returns the amount to change pc by
/* TODO:
    Ops BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BVC, BVS, CMP, CPX, CPY, JMP, JSR, PHA, PHP, PLA, PLP, RTI, RTS
    Addressing Modes (IND, [X, Y]) and (IND), [X, Y]
    Check for if ops set other flags
*/
static byte instruction(byte opcode, byte ops[]) {
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
            ORA ZP
        }

        case 0x06: // ASL (Shift Left One Bit (Memory or Accumulator)) ZP
        {
            ASL ZP
        }

        case 0x09: // ORA ("OR" Memory with Accumulator) IMM
        {
            ORA IMM
        }

        case 0x0A: // ASL (Shift Left One Bit (Memory or Accumulator)) Accum
        {
            ASL Accum
        }

        case 0x0D: // ORA ("OR" Memory with Accumulator) ABS
        {
            ORA ABS
        }

        case 0x0E: // ASL (Shift Left One Bit (Memory or Accumulator)) ABS
        {
            ASL ABS
        }

        case 0x11: // ORA (IND), Y
        {
            break;
        }

        case 0x15: // ORA ("OR" Memory with Accumulator) ZP, X
        {
            ORA ZP_X
        }

        case 0x16: // ASL (Shift Left One Bit (Memory or Accumulator)) ZP, X
        {
            ASL ZP_X
        }

        case 0x18: // CLC (Clear Carry Flag) Implied
        {
            CLC Implied
        }

        case 0x19: // ORA ("OR" Memory with Accumulator) ABS, Y
        {
            ORA ABS_Y
        }

        case 0x1D: // ORA ("OR" Memory with Accumulator) ABS, X
        {
            ORA ABS_X
        }

        case 0x1E: // ASL (Shift Left One Bit (Memory or Accumulator)) ABS, X
        {
            ASL ABS_X
        }

        case 0x21: // AND (IND, X)
        {
            break;
        }

        case 0x25: // AND ("AND" Memory with Accumulator) ZP
        {
            AND ZP
        }

        case 0x26: // ROL (Rotate One Bit Left (Memory or Accumulator)) ZP
        {
            ROL ZP
        }

        case 0x29: // AND ("AND" Memory with Accumulator) IMM
        {
            AND IMM
        }

        case 0x2A: // ROL (Rotate One Bit Left (Memory or Accumulator)) Accum
        {
            ROL Accum
        }

        case 0x2D: // AND ("AND" Memory with Accumulator) ABS
        {
            AND ABS
        }

        case 0x2E: // ROL (Rotate One Bit Left (Memory or Accumulator)) ABS
        {
            ROL ABS
        }
        
        case 0x31: // AND (IND), Y
        {
            break;
        }

        case 0x35: // AND ("AND" Memory with Accumulator) ZP, X
        {
            AND ZP_X
        }

        case 0x36: // ROL (Rotate One Bit Left (Memory or Accumulator)) ZP, X
        {
            ROL ZP_X
        }

        case 0x38: // SEC (Set Carry Flag) Implied
        {
            SEC Implied
        }

        case 0x39: // AND ("AND" Memory with Accumulator) ABS, Y
        {
            AND ABS_Y
        }

        case 0x3D: // AND ("AND" Memory with Accumulator) ABS, X
        {
            AND ABS_X
        }

        case 0x3E: // ROL (Rotate One Bit Left (Memory or Accumulator)) ABS, X
        {
            ROL ABS_X
        }

        case 0x41: // EOR (IND, X)
        {
            break;
        }

        case 0x45: // EOR ("Exclusive-OR" Memory with Accumulator) ZP
        {
            EOR ZP
        }

        case 0x46: // LSR (Shift One Bit Right (Memory or Accumulator)) ZP
        {
            LSR ZP
        }

        case 0x49: // EOR ("Exclusive-OR" Memory with Accumulator) IMM
        {
            EOR IMM
        }

        case 0x4A: // LSR (Shift One Bit Right (Memory or Accumulator)) Accum
        {
            LSR Accum
        }

        case 0x4D: // EOR ("Exclusive-OR" Memory with Accumulator) ABS
        {
            EOR ABS
        }

        case 0x4E: // LSR (Shift One Bit Right (Memory or Accumulator)) ABS
        {
            LSR ABS
        }

        case 0x51: // EOR (IND), Y
        {
            break;
        }

        case 0x55: // EOR ("Exclusive-OR" Memory with Accumulator) ZP, X
        {
            EOR ZP_X
        }

        case 0x56: // LSR (Shift One Bit Right (Memory or Accumulator)) ZP, X
        {
            LSR ZP_X
        }

        case 0x58: // CLI (Clear Interrupt Disable Bit) Implied
        {
            CLI Implied
        }

        case 0x59: // EOR ("Exclusive-OR" Memory with Accumulator) ABS, Y
        {
            EOR ABS_Y
        }

        case 0x5D: // EOR ("Exclusive-OR" Memory with Accumulator) ABS, X
        {
            EOR ABS_X
        }

        case 0x5E: // LSR (Shift One Bit Right (Memory or Accumulator)) ABS, X
        {
            LSR ABS_X
        }

        case 0x61: // ADC (IND, X)
        {
            break;
        }

        case 0x65: // ADC (Add Memory to Accumulator with Carry) ZP
        {
            ADC ZP
        }

        case 0x66: // ROR (Rotate One Bit Right (Memory or Accumulator)) ZP
        {
            ROR ZP
        }

        case 0x69: // ADC (Add Memory to Accumulator with Carry) IMM
        {
            ADC IMM
        }

        case 0x6A: // ROR (Rotate One Bit Right (Memory or Accumulator)) Accum
        {
            ROR Accum
        }

        case 0x6D: // ADC (Add Memory to Accumulator with Carry) ABS
        {
            ADC ABS
        }

        case 0x6E: // ROR (Rotate One Bit Right (Memory or Accumulator)) ABS
        {
            ROR ABS
        }

        case 0x71: // ADC (IND), Y
        {
            break;
        }

        case 0x75: // ADC (Add Memory to Accumulator with Carry) ZP, X
        {
            ADC ZP_X
        }

        case 0x76: // ROR (Rotate One Bit Right (Memory or Accumulator)) ZP, X
        {
            ROR ZP_X
        }

        case 0x78: // SEI (Set Interrupt Disable Status) Implied
        {
            SEI Implied
        }

        case 0x79: // ADC (Add Memory to Accumulator with Carry) ABS, Y
        {
            ADC ABS_Y
        }

        case 0x7D: // ADC (Add Memory to Accumulator with Carry) ABS, X
        {
            ADC ABS_X
        }

        case 0x7E: // ROR (Rotate One Bit Right (Memory or Accumulator)) ABS, X
        {
            ROR ABS_X
        }

        case 0x81: // STA (IND, X)
        {
            break;
        }

        case 0x84: // STY (Store Index Y in Memory) ZP
        {
            STY ZP
        }

        case 0x85: // STA (Store Accumulator in Memory) ZP
        {
            STA ZP
        }
        
        case 0x86: // STX (Store Index X in Memory) ZP
        {
            STX ZP
        }
        
        case 0x88: // DEY (Decrement Index Y by One) Implied
        {
            DEY Implied
        }

        case 0x8A: // TXA (Transfer Index X to Accumulator) Implied
        {
            TXA Implied
        }

        case 0x8C: // STY (Store Index Y in Memory) ABS
        {
            STY ABS
        }

        case 0x8D: // STA (Store Accumulator in Memory) ABS
        {
            STA ABS
        }

        case 0x8E: // STX (Store Index X in Memory) ABS
        {
            STX ABS
        }

        case 0x91: // STA (IND), Y
        {
            break;
        }

        case 0x94: // STY (Store Index Y in Memory) ZP, X
        {
            STY ZP_X
        }

        case 0x95: // STA (Store Accumulator in Memory) ZP, X
        {
            STA ZP_X
        }

        case 0x96: // STX (Store Index X in Memory) ZP, Y
        {
            STX ZP_Y
        }

        case 0x98: // TYA (Transfer Index Y to Accumulator) Implied
        {
            TYA Implied
        }

        case 0x99: // STA (Store Accumulator in Memory) ABS, Y
        {
            STA ABS_Y
        }

        case 0x9A: // TXS Implied
        {
            break;
        }

        case 0x9D: // STA (Store Accumulator in Memory) ABS, X
        {
            STA ABS_X
        }
        
        case 0xA0: // LDY (Load Index Y with Memory) IMM
        {
            LDY IMM
        }

        case 0xA1: // LDA (IND, X)
        {
            break;
        }

        case 0xA2: // LDX (Load Index X with Memory) IMM
        {
            LDX IMM
        }

        case 0xA4: // LDY (Load Index Y with Memory) ZP
        {
            LDY ZP
        }
        
        case 0xA5: // LDA (Load Accumulator with Memory) ZP
        {
            LDA ZP
        }

        case 0xA6: // LDX (Load Index X with Memory) ZP
        {
            LDX ZP
        }

        case 0xA8: // TAY (Transfer Accumulator to Index Y) Implied
        {
            TAY Implied
        }

        case 0xA9: // LDA (Load Accumulator with Memory) IMM
        {
            LDA IMM
        }

        case 0xAA: // TAX (Transfer Accumulator to Index X) Implied
        {
            TAX Implied
        }

        case 0xAC: // LDY (Load Index Y with Memory) ABS
        {
            LDY ABS
        }

        case 0xAD: // LDA (Load Accumulator with Memory) ABS
        {
            LDA ABS
        }

        case 0xAE: // LDX (Load Index X with Memory) ABS
        {
            LDX ABS
        }

        case 0xB1: // LDA (IND), Y
        {
            break;
        }

        case 0xB4: // LDY (Load Index Y with Memory) ZP, X
        {
            LDY ZP_X
        }

        case 0xB5: // LDA (Load Accumulator with Memory) ZP, X
        {
            LDA ZP_X
        }

        case 0xB6: // LDX (Load Index X with Memory) ZP, Y
        {
            LDX ZP_Y
        }
        
        case 0xB8: // CLV (Clear Overflow Flag) Implied
        {
            CLV Implied
        }

        case 0xB9: // LDA (Load Accumulator with Memory) ABS, Y
        {
            LDA ABS_Y
        }

        case 0xBA: // TSX Implied
        {
            break;
        }

        case 0xBC: // LDY (Load Index Y with Memory) ABS, X
        {
            LDY ABS_X
        }

        case 0xBD: // LDA (Load Accumulator with Memory) ABS, X
        {
            LDA ABS_X
        }

        case 0xBE: // LDX (Load Index X with Memory) ABS, Y
        {
            LDX ABS_Y
        }

        case 0xC6: // DEC (Decrement Memory by One) ZP
        {
            DEC ZP
        }

        case 0xC8: // INY (Increment Index Y by One) Implied
        {
            INY Implied
        }

        case 0xCA: // DEX (Decrement Index X by One) Implied
        {
            DEX Implied
        }

        case 0xCE: // DEC (Decrement Memory by One) ABS
        {
            DEC ABS
        }

        case 0xD6: // DEC (Decrement Memory by One) ZP, X
        {
            DEC ZP_X
        }

        case 0xD8: // CLD (Clear Decimal Mode) Implied
        {
            CLD Implied
        }

        case 0xDE: // DEC (Decrement Memory by One) ABS, X
        {
            DEC ABS_X
        }

        case 0xE1: // SBC (IND, X)
        {
            break;
        }

        case 0xE5: // SBC (Subtract Memory from Accumulator with Borrow) ZP
        {
            SBC ZP
        }

        case 0xE6: // INC (Increment Memory by One) ZP
        {
            INC ZP
        }

        case 0xE8: // INX (Increment Index X by One) Implied
        {
            INX Implied
        }

        case 0xE9: // SBC (Subtract Memory from Accumulator with Borrow) IMM
        {
            SBC IMM
        }

        case 0xEA: // NOP (No Operation) Implied
        {
            NOP Implied
        }

        case 0xED: // SBC (Subtract Memory from Accumulator with Borrow) ABS
        {
            SBC ABS
        }
        
        case 0xEE: // INC (Increment Memory by One) ABS
        {
            INC ABS
        }

        case 0xF1: // SBC (IND), Y
        {
            break;
        }

        case 0xF5: // SBC (Subtract Memory from Accumulator with Borrow) ZP, X
        {
            SBC ZP_X
        }

        case 0xF6: // INC (Increment Memory by One) ZP, X
        {
            INC ZP_X
        }

        case 0xF8: // SED (Set Decimal Mode) Implied
        {
            SED Implied
        }

        case 0xF9: // SBC (Subtract Memory from Accumulator with Borrow) ABS, Y
        {
            SBC ABS_Y
        }

        case 0xFD: // SBC (Subtract Memory from Accumulator with Borrow) ABS, X
        {
            SBC ABS_X
        }

        case 0xFE: // INC (Increment Memory by One) ABS, X
        {
            INC ABS_X
        }
    }

    // Break if not returned

    printf("Broke on opcode %02X\n", opcode);

    return BRK_MOVE;
}
