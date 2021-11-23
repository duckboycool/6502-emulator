#include <stdio.h>

/* ops.h
  Contains operations and register/memory values.
*/

bool broken = false;

#ifdef _WIN32
#define EXESTRING "6502.exe"

#include <windows.h>

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            broken = true;
            return TRUE;

        default:
            return FALSE;
    }
}
#else
#define EXESTRING "6502"
#define byte unsigned char

#include <signal.h>

void int_handler(int s) {
    broken = true;
}
#endif

#define byte2 unsigned short

// Absolute address
#define ABS_ADDR ops[1] * 0x100 + ops[0]

using std::string;

// Create memory and registers
byte memory[0x10000] = {};

// Accumulator, x and y registers
byte a, x, y;

// Stack pointer
byte sp = 0xFF;

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
// Break when hitting a software break (BRK)
bool brk_stop = false;
// Print value if on printing address and enabled
byte* print_ptr = &memory[0xFFF9];
bool print_out = false;

string endprint = ""; // Printed out after program stops

void st_print(byte* addr, int val) {
    if (addr == print_ptr && print_out) {
        // Print at end if printing memory or instructions (so no overlap), otherwise, print now
        if (mem_print || ins_print) {
            endprint += (char)val;
            return;
        }

        printf("%c", (char)val);
    }
}

// Set negative and zero flags based off of input value
void set_nz(byte val) {
    sr.n = val & 0b10000000;
    sr.z = !val;
}

// Define addressing modes
#define Accum (a);\
    return 0x01;               // Accumulator as an argument
#define IMM (ops[0]);\
    return 0x02;               // Literal first value as an argument
#define ABS (memory[ABS_ADDR]);\
    return 0x03;               // Value at address of next two bytes
#define ZP (memory[ops[0]]);\
    return 0x02;               // Value at address of next byte with high byte (page) of 0
#define ZP_X \
  (memory[(byte)(ops[0] + x)]);\
    return 0x02;               // Value at address of next byte with high byte 0, indexed to x
#define ZP_Y \
  (memory[(byte)(ops[0] + y)]);\
    return 0x02;               // ^, indexed to y
#define ABS_X \
  (memory[(byte2)(ABS_ADDR + x)]);\
    return 0x03;               // Value at address of next two bytes, indexed to x
#define ABS_Y \
  (memory[(byte2)(ABS_ADDR + y)]);\
    return 0x03;               // ^, indexed to y
#define Implied ();\
    return 0x01;               // No arguments (implied from instruction)
#define Relative ((signed char)ops[0]);\
    return 0x02;               // First value as argument (interpreted as -128 to 127)
#define IND_X (memory[memory[(byte)(ops[0] + x + 1)] * 0x100 + memory[(byte)(ops[0] + x)]]);\
    return 0x02;               // Value at indexed indirect x memory location
#define IND_Y (memory[(byte2)(memory[(byte)(ops[0] + 1)] * 0x100 + y + memory[ops[0]])]);\
    return 0x02;               // Value at indirect indexed y memory location

// Define instructions
void ORA(byte val) {
    set_nz(a = a | val);
}

void ASL(byte& addr) {
    sr.c = addr / 0x80;

    set_nz(addr = addr * 0b10);
}

void PHP() {
    memory[0x100 + sp] = sr.val();
    sp--;
}

void BPL(byte val) {
    if (!sr.n) pc += val;
}

void CLC() {
    sr.c = false;
}

void JSR(byte val) {
    memory[0x100 + sp] = (pc + 2) / 0x100;
    sp--;
    memory[0x100 + sp] = (pc + 2) % 0x100;
    sp--;

    pc = memory[pc + 2] * 0x100 + memory[pc + 1] - 3; // FIXME: Possibly not ideal
}

void ROL(byte& addr) {
    bool bit = addr / 0x80;

    set_nz(addr = addr * 0b10 + sr.c);

    sr.c = bit;
}

void AND(byte val) {
    set_nz(a = a & val);
}

void BIT(byte val) {
    sr.n = 0b10000000 & val;
    sr.v = 0b01000000 & val;
    
    sr.z = !(a & val);
}

void PLP() {
    sp++;
    byte val = memory[0x100 + sp];

    sr.n = val & 0b10000000;
    sr.v = val & 0b01000000;
    sr._ = val & 0b00100000;
    sr.b = val & 0b00010000;
    sr.d = val & 0b00001000;
    sr.i = val & 0b00000100;
    sr.z = val & 0b00000010;
    sr.c = val & 0b00000001;
}

void BMI(byte val) {
    if (sr.n) pc += val;
}

void SEC() {
    sr.c = true;
}

void RTI() {
    PLP();

    sp++;
    byte2 val = memory[0x100 + sp];
    sp++;
    val += 0x100 * memory[0x100 + sp];

    pc = val - 1;
}

void EOR(byte val) {
    set_nz(a = a ^ val);
}

void LSR(byte& addr) {
    sr.c = addr % 0b10;

    set_nz(addr = addr / 0b10);
}

void PHA() {
    memory[0x100 + sp] = a;
    sp--;
}

void BVC(signed char val) {
    if (!sr.v) pc += val;
}

void CLI() {
    sr.i = false;
}

void RTS() {
    sp++;
    byte2 val = memory[0x100 + sp];
    sp++;
    val += 0x100 * memory[0x100 + sp];

    pc = val;
}

void ADC(byte val) {
    short sum = (signed char)a + (signed char)val;
    sr.c = (val + sr.c + a > 0xFF);
    sr.v = sum < -0x80 || sum >= 0x80;

    set_nz(a = a + val + sr.c);    
}

void ROR(byte& addr) {
    bool bit = addr % 0b10;

    set_nz(addr = 0x80 * sr.c + addr / 0b10);

    sr.c = bit;
}

void PLA() {
    sp++;
    set_nz(a = memory[0x100 + sp]);
}

void BVS(signed char val) {
    if (sr.v) pc += val;
}

void SEI() {
    sr.i = true;
}

void STA(byte& addr) {
    st_print(&addr, a);

    addr = a;
}

void STY(byte& addr) {
    st_print(&addr, y);

    addr = y;
}

void STX(byte& addr) {
    st_print(&addr, x);

    addr = x;
}

void DEY() {
    set_nz(--y);
}

void TXA() {
    set_nz(a = x);
}

void BCC(signed char val) {
    if (!sr.c) pc += val;
}

void TYA() {
    set_nz(a = y);
}

void TXS() {
    sp = x;
}

void LDY(byte val) {
    set_nz(y = val);
}

void LDA(byte val) {
    set_nz(a = val);
}

void LDX(byte val) {
    set_nz(x = val);
}

void TAY() {
    set_nz(y = a);
}

void TAX() {
    set_nz(x = a);
}

void BCS(signed char val) {
    if (sr.c) pc += val;
}

void CLV() {
    sr.v = false;
}

void TSX() {
    set_nz(x = sp);
}

void CPY(byte val) {
    set_nz(y - val);
    sr.c = y >= val;
}

void CMP(byte val) {
    set_nz(a - val);
    sr.c = a >= val;
}

void DEC(byte& addr) {
    set_nz(--addr);
}

void INY() {
    set_nz(++y);
}

void DEX() {
    set_nz(--x);
}

void BNE(signed char val) {
    if (!sr.z) pc += val;
}

void CLD() {
    sr.d = false;
}

void CPX(byte val) {
    set_nz(x - val);
    sr.c = x >= val;
}

void SBC(byte val) {
    short dif = (signed char)a - (signed char)val;
    sr.c = (a >= val + !sr.c);
    sr.v = dif < -0x80 || dif >= 0x80;

    set_nz(a = a - val - !sr.c);
}

void INC(byte& addr) {
    set_nz(++addr);
}

void INX() {
    set_nz(++x);
}

void NOP() {
    
}

void BEQ(signed char val) {
    if (sr.z) pc += val;
}

void SED() {
    sr.d = true;
}

// Function that executes instructions and returns the amount to change pc by
/* TODO:
    Implement overflow flag on ADC and SBC
    Properly set values on routines
    Implement decimal mode
*/
static byte instruction(byte opcode, byte ops[]) {
    switch (opcode) {
        case 0x00: // BRK (Force Break) Implied
            memory[0x100 + sp] = (pc + 2) / 0x100;
            sp--;
            memory[0x100 + sp] = (pc + 2) % 0x100;
            sp--;

            PHP();
            sr.i = true;

            pc = 0x100 * memory[0xFFFF] + memory[0xFFFE];

            return brk_stop ? 0x00 : BRK_MOVE;

        case 0x01: // ORA ("OR" Memory with Accumulator) (IND, X)
            ORA IND_X

        case 0x05: // ORA ("OR" Memory with Accumulator) ZP
            ORA ZP

        case 0x06: // ASL (Shift Left One Bit (Memory or Accumulator)) ZP
            ASL ZP

        case 0x08: // PHP (Push Processor Status on Stack) Implied
            PHP Implied

        case 0x09: // ORA ("OR" Memory with Accumulator) IMM
            ORA IMM

        case 0x0A: // ASL (Shift Left One Bit (Memory or Accumulator)) Accum
            ASL Accum

        case 0x0D: // ORA ("OR" Memory with Accumulator) ABS
            ORA ABS

        case 0x0E: // ASL (Shift Left One Bit (Memory or Accumulator)) ABS
            ASL ABS

        case 0x10: // BPL (Branch on Result Plus) Relative
            BPL Relative

        case 0x11: // ORA ("OR" Memory with Accumulator) (IND), Y
            ORA IND_Y

        case 0x15: // ORA ("OR" Memory with Accumulator) ZP, X
            ORA ZP_X

        case 0x16: // ASL (Shift Left One Bit (Memory or Accumulator)) ZP, X
            ASL ZP_X

        case 0x18: // CLC (Clear Carry Flag) Implied
            CLC Implied

        case 0x19: // ORA ("OR" Memory with Accumulator) ABS, Y
            ORA ABS_Y

        case 0x1D: // ORA ("OR" Memory with Accumulator) ABS, X
            ORA ABS_X

        case 0x1E: // ASL (Shift Left One Bit (Memory or Accumulator)) ABS, X
            ASL ABS_X

        case 0x20: // JSR (Jump to New Location Saving Return Address) ABS
            JSR ABS

        case 0x21: // AND ("AND" Memory with Accumulator) (IND, X)
            AND IND_X

        case 0x24: // BIT (Test Bits in Memory with Accumulator) ZP
            BIT ZP

        case 0x25: // AND ("AND" Memory with Accumulator) ZP
            AND ZP

        case 0x26: // ROL (Rotate One Bit Left (Memory or Accumulator)) ZP
            ROL ZP

        case 0x28: // PLP (Pull Processor Status from Stack) Implied
            PLP Implied

        case 0x29: // AND ("AND" Memory with Accumulator) IMM
            AND IMM

        case 0x2A: // ROL (Rotate One Bit Left (Memory or Accumulator)) Accum
            ROL Accum

        case 0x2C: // BIT (Test Bits in Memory with Accumulator) ABS
            BIT ABS

        case 0x2D: // AND ("AND" Memory with Accumulator) ABS
            AND ABS

        case 0x2E: // ROL (Rotate One Bit Left (Memory or Accumulator)) ABS
            ROL ABS

        case 0x30: // BMI (Branch on Result Minus) Relative
            BMI Relative
        
        case 0x31: // AND ("AND" Memory with Accumulator) (IND), Y
            AND IND_Y

        case 0x35: // AND ("AND" Memory with Accumulator) ZP, X
            AND ZP_X

        case 0x36: // ROL (Rotate One Bit Left (Memory or Accumulator)) ZP, X
            ROL ZP_X

        case 0x38: // SEC (Set Carry Flag) Implied
            SEC Implied

        case 0x39: // AND ("AND" Memory with Accumulator) ABS, Y
            AND ABS_Y

        case 0x3D: // AND ("AND" Memory with Accumulator) ABS, X
            AND ABS_X

        case 0x3E: // ROL (Rotate One Bit Left (Memory or Accumulator)) ABS, X
            ROL ABS_X

        case 0x40: // RTI (Return from Interrupt) Implied
            RTI Implied

        case 0x41: // EOR ("Exclusive-OR" Memory with Accumulator) (IND, X)
            EOR IND_X

        case 0x45: // EOR ("Exclusive-OR" Memory with Accumulator) ZP
            EOR ZP

        case 0x46: // LSR (Shift One Bit Right (Memory or Accumulator)) ZP
            LSR ZP

        case 0x48: // PHA (Push Accumulator on Stack) Implied
            PHA Implied

        case 0x49: // EOR ("Exclusive-OR" Memory with Accumulator) IMM
            EOR IMM

        case 0x4A: // LSR (Shift One Bit Right (Memory or Accumulator)) Accum
            LSR Accum

        case 0x4C: // JMP (Jump to New Location) ABS
            pc = ABS_ADDR;
            return 0x00;

        case 0x4D: // EOR ("Exclusive-OR" Memory with Accumulator) ABS
            EOR ABS

        case 0x4E: // LSR (Shift One Bit Right (Memory or Accumulator)) ABS
            LSR ABS

        case 0x50: // BVC (Branch on Overflow Clear) Relative
            BVC Relative

        case 0x51: // EOR ("Exclusive-OR" Memory with Accumulator) (IND), Y
            EOR IND_Y

        case 0x55: // EOR ("Exclusive-OR" Memory with Accumulator) ZP, X
            EOR ZP_X

        case 0x56: // LSR (Shift One Bit Right (Memory or Accumulator)) ZP, X
            LSR ZP_X

        case 0x58: // CLI (Clear Interrupt Disable Bit) Implied
            CLI Implied

        case 0x59: // EOR ("Exclusive-OR" Memory with Accumulator) ABS, Y
            EOR ABS_Y

        case 0x5D: // EOR ("Exclusive-OR" Memory with Accumulator) ABS, X
            EOR ABS_X

        case 0x5E: // LSR (Shift One Bit Right (Memory or Accumulator)) ABS, X
            LSR ABS_X

        case 0x60: // RTS (Return from Subroutine) Implied
            RTS Implied

        case 0x61: // ADC (Add Memory to Accumulator with Carry) (IND, X)
            ADC IND_X

        case 0x65: // ADC (Add Memory to Accumulator with Carry) ZP
            ADC ZP

        case 0x66: // ROR (Rotate One Bit Right (Memory or Accumulator)) ZP
            ROR ZP

        case 0x68: // PLA (Pull Accumulator from Stack) Implied
            PLA Implied

        case 0x69: // ADC (Add Memory to Accumulator with Carry) IMM
            ADC IMM

        case 0x6A: // ROR (Rotate One Bit Right (Memory or Accumulator)) Accum
            ROR Accum

        case 0x6C: // JMP (Jump to New Location) Indirect
            pc = memory[ABS_ADDR] + 0x100 * memory[ops[1] * 0x100 + (byte)(ops[0] + 1)];
            return 0x00;

        case 0x6D: // ADC (Add Memory to Accumulator with Carry) ABS
            ADC ABS

        case 0x6E: // ROR (Rotate One Bit Right (Memory or Accumulator)) ABS
            ROR ABS

        case 0x70: // BVS (Branch on Overflow Set) Relative
            BVS Relative

        case 0x71: // ADC (Add Memory to Accumulator with Carry) (IND), Y
            ADC IND_Y

        case 0x75: // ADC (Add Memory to Accumulator with Carry) ZP, X
            ADC ZP_X

        case 0x76: // ROR (Rotate One Bit Right (Memory or Accumulator)) ZP, X
            ROR ZP_X

        case 0x78: // SEI (Set Interrupt Disable Status) Implied
            SEI Implied

        case 0x79: // ADC (Add Memory to Accumulator with Carry) ABS, Y
            ADC ABS_Y

        case 0x7D: // ADC (Add Memory to Accumulator with Carry) ABS, X
            ADC ABS_X

        case 0x7E: // ROR (Rotate One Bit Right (Memory or Accumulator)) ABS, X
            ROR ABS_X

        case 0x81: // STA (Store Accumulator in Memory) (IND, X)
            STA IND_X

        case 0x84: // STY (Store Index Y in Memory) ZP
            STY ZP

        case 0x85: // STA (Store Accumulator in Memory) ZP
            STA ZP
        
        case 0x86: // STX (Store Index X in Memory) ZP
            STX ZP
        
        case 0x88: // DEY (Decrement Index Y by One) Implied
            DEY Implied

        case 0x8A: // TXA (Transfer Index X to Accumulator) Implied
            TXA Implied

        case 0x8C: // STY (Store Index Y in Memory) ABS
            STY ABS

        case 0x8D: // STA (Store Accumulator in Memory) ABS
            STA ABS

        case 0x8E: // STX (Store Index X in Memory) ABS
            STX ABS

        case 0x90: // BCC (Branch on Carry Clear) Relative
            BCC Relative

        case 0x91: // STA (Store Accumulator in Memory) (IND), Y
            STA IND_Y

        case 0x94: // STY (Store Index Y in Memory) ZP, X
            STY ZP_X

        case 0x95: // STA (Store Accumulator in Memory) ZP, X
            STA ZP_X

        case 0x96: // STX (Store Index X in Memory) ZP, Y
            STX ZP_Y

        case 0x98: // TYA (Transfer Index Y to Accumulator) Implied
            TYA Implied

        case 0x99: // STA (Store Accumulator in Memory) ABS, Y
            STA ABS_Y

        case 0x9A: // TXS (Transfer Index X to Stack Register) Implied
            TXS Implied

        case 0x9D: // STA (Store Accumulator in Memory) ABS, X
            STA ABS_X
        
        case 0xA0: // LDY (Load Index Y with Memory) IMM
            LDY IMM

        case 0xA1: // LDA (Load Accumulator with Memory) (IND, X)
            LDA IND_X

        case 0xA2: // LDX (Load Index X with Memory) IMM
            LDX IMM

        case 0xA4: // LDY (Load Index Y with Memory) ZP
            LDY ZP
        
        case 0xA5: // LDA (Load Accumulator with Memory) ZP
            LDA ZP

        case 0xA6: // LDX (Load Index X with Memory) ZP
            LDX ZP

        case 0xA8: // TAY (Transfer Accumulator to Index Y) Implied
            TAY Implied

        case 0xA9: // LDA (Load Accumulator with Memory) IMM
            LDA IMM

        case 0xAA: // TAX (Transfer Accumulator to Index X) Implied
            TAX Implied

        case 0xAC: // LDY (Load Index Y with Memory) ABS
            LDY ABS

        case 0xAD: // LDA (Load Accumulator with Memory) ABS
            LDA ABS

        case 0xAE: // LDX (Load Index X with Memory) ABS
            LDX ABS

        case 0xB0: // BCS (Branch on Carry Set) Relative
            BCS Relative

        case 0xB1: // LDA (Load Accumulator with Memory) (IND), Y
            LDA IND_Y

        case 0xB4: // LDY (Load Index Y with Memory) ZP, X
            LDY ZP_X

        case 0xB5: // LDA (Load Accumulator with Memory) ZP, X
            LDA ZP_X

        case 0xB6: // LDX (Load Index X with Memory) ZP, Y
            LDX ZP_Y
        
        case 0xB8: // CLV (Clear Overflow Flag) Implied
            CLV Implied

        case 0xB9: // LDA (Load Accumulator with Memory) ABS, Y
            LDA ABS_Y

        case 0xBA: // TSX (Transfer Stack Pointer to Index X) Implied
            TSX Implied

        case 0xBC: // LDY (Load Index Y with Memory) ABS, X
            LDY ABS_X

        case 0xBD: // LDA (Load Accumulator with Memory) ABS, X
            LDA ABS_X

        case 0xBE: // LDX (Load Index X with Memory) ABS, Y
            LDX ABS_Y

        case 0xC0: // CPY (Compare Memory and Index Y) IMM
            CPY IMM

        case 0xC1: // CMP (Compare Memory and Accumulator) (IND, X)
            CMP IND_X

        case 0xC4: // CPY (Compare Memory and Index Y) ZP
            CPY ZP

        case 0xC5: // CMP (Compare Memory and Accumulator) ZP
            CMP ZP

        case 0xC6: // DEC (Decrement Memory by One) ZP
            DEC ZP

        case 0xC8: // INY (Increment Index Y by One) Implied
            INY Implied

        case 0xC9: // CMP (Compare Memory and Accumulator) IMM
            CMP IMM

        case 0xCA: // DEX (Decrement Index X by One) Implied
            DEX Implied

        case 0xCC: // CPY (Compare Memory and Index Y) ABS
            CPY ABS

        case 0xCD: // CMP (Compare Memory and Accumulator) ABS
            CMP ABS

        case 0xCE: // DEC (Decrement Memory by One) ABS
            DEC ABS

        case 0xD0: // BNE (Branch on Result not Zero) Relative
            BNE Relative

        case 0xD1: // CMP (Compare Memory and Accumulator) (IND), Y
            CMP IND_Y

        case 0xD5: // CMP (Compare Memory and Accumulator) ZP, X
            CMP ZP_X

        case 0xD6: // DEC (Decrement Memory by One) ZP, X
            DEC ZP_X

        case 0xD8: // CLD (Clear Decimal Mode) Implied
            CLD Implied

        case 0xD9: // CMP (Compare Memory and Accumulator) ABS, Y
            CMP ABS_Y

        case 0xDD: // CMP (Compare Memory and Accumulator) ABS, X
            CMP ABS_X

        case 0xDE: // DEC (Decrement Memory by One) ABS, X
            DEC ABS_X

        case 0xE0: // CPX (Compare Memory and Index X) IMM
            CPX IMM

        case 0xE1: // SBC (Subtract Memory from Accumulator with Borrow) (IND, X)
            SBC IND_X

        case 0xE4: // CPX (Compare Memory and Index X) ZP
            CPX ZP

        case 0xE5: // SBC (Subtract Memory from Accumulator with Borrow) ZP
            SBC ZP

        case 0xE6: // INC (Increment Memory by One) ZP
            INC ZP

        case 0xE8: // INX (Increment Index X by One) Implied
            INX Implied

        case 0xE9: // SBC (Subtract Memory from Accumulator with Borrow) IMM
            SBC IMM

        case 0xEA: // NOP (No Operation) Implied
            NOP Implied

        case 0xEC: // CPX (Compare Memory and Index X) ABS
            CPX ABS

        case 0xED: // SBC (Subtract Memory from Accumulator with Borrow) ABS
            SBC ABS
        
        case 0xEE: // INC (Increment Memory by One) ABS
            INC ABS

        case 0xF0: // BEQ (Branch on Result Zero) Relative
            BEQ Relative

        case 0xF1: // SBC (Subtract Memory from Accumulator with Borrow) (IND), Y
            SBC IND_Y

        case 0xF5: // SBC (Subtract Memory from Accumulator with Borrow) ZP, X
            SBC ZP_X

        case 0xF6: // INC (Increment Memory by One) ZP, X
            INC ZP_X

        case 0xF8: // SED (Set Decimal Mode) Implied
            SED Implied

        case 0xF9: // SBC (Subtract Memory from Accumulator with Borrow) ABS, Y
            SBC ABS_Y

        case 0xFD: // SBC (Subtract Memory from Accumulator with Borrow) ABS, X
            SBC ABS_X

        case 0xFE: // INC (Increment Memory by One) ABS, X
            INC ABS_X
    }

    // Break if not returned
    printf("Broke on opcode %02X\n", opcode);

    return BRK_MOVE;
}
