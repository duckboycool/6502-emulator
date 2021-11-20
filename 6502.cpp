#include <iostream>
#include <fstream>
#include <sstream>

#include "ops.h"

#ifdef _WIN32
#define EXESTRING "6502.exe"
#else
#define EXESTRING "6502"
#endif

#define VERSIONSTRING "v0.2.1-dev"

// Function to get ascii representation of memory for printout
string ascii(byte2 memaddr, int size) {
    string out = "";

    for (int val = 0; val < size; val++) {
        char memval = memory[memaddr + val];
        if (memval >= 0x20 && memval <= 0x7e) { // From spacebar to tilde
            out += memval;
        } else {
            out += '.'; // Period for other characters
        }
    }

    return out;
}

// TODO: Possible refactoring
int main(int argc, char** argv) {
    // Handle options and file
    char* file = NULL;

    int memstart = 0x0000;
    int rows = 8;
    int rowsize = 16;
    int romstart = 0x0000;

    string codestring;

    for (int i = 1; i < argc; i++) {
        // Check for options
        if (argv[i][0] == '-') {
            if (argv[i] == string("-?")) {
                printf(
                    "6502 %s HELP\n"
                    "  Usage:\n"
                    "    %s [-options | --flags] [-o code] [-f] {file}\n"
                    "\n"
                    "  Options:\n"
                    "    -?        Display help and stop execution (this message).\n"
                    "    -sa {x}   Set the starting address for the ROM. Prefix with \"0x\" for hex input (default 0x0000).\n"
                    "    -mr {x}   Set the number of rows to print out when displaying memory (default 8).\n"
                    "    -ml {x}   Set the length of rows to print out when displaying memory (default 16).\n"
                    "    -ms {x}   Set the starting value of memory to print rows from. Prefix with \"0x\" for hex input (default 0).\n"
                    "    --m       Disable the printing of memory once hitting a break (default true).\n"
                    "    --a       Disable ascii representation of printed memory (default true).\n"
                    "    --i       Disable the printing of instructions while executing (default true).\n"
                    "    --p       Enable the printing address (will only print at end if --m or --i) (default false).\n"
                    "    -pa {x}   Set value of printing address. Prefix with \"0x\" for hex input (default 0xFFF9).\n"
                    "    -o {x}    Directly input assembled machine code as string and ignore rest of input.\n"
                    "    -f {x}    Explicitly select the input file to be executed and ignore rest of input (default last arg).\n",
                    VERSIONSTRING,
                    EXESTRING
                    );
                return -1;
            }

            else if (argv[i] == string("--m")) {
                mem_print = false;
            }

            else if (argv[i] == string("--a")) {
                asc_print = false;
            }

            else if (argv[i] == string("--i")) {
                ins_print = false;
            }

            else if (argv[i] == string("--p")) {
                print_out = true;
            }

            else if (argv[i] == string("-sa")) {
                if (argc == i + 1) {
                    printf("-sa requires an argument.\n");
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
                
                romstart = x;

                i++;
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

            else if (argv[i] == string("-ml")) {
                if (argc == i + 1) {
                    printf("-ml requires an argument.\n");
                    return 1;
                }

                // Parse int
                int x = 0;
                stringstream val(argv[i + 1]);

                val >> x;
                rowsize = x;

                i++;
            }

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
                
                memstart = x;

                i++;
            }

            else if (argv[i] == string("-pa")) {
                if (argc == i + 1) {
                    printf("-pa requires an argument.\n");
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
                
                print_ptr = &memory[x];

                i++;
            }

            else if (argv[i] == string("-o")) {
                if (argc == i + 1) {
                    printf("-o requires an argument.\n");
                    return 1;
                }

                codestring = argv[i + 1];
                break;
            }

            else if (argv[i] == string("-f")) {
                if (argc == i + 1) {
                    printf("-f requires an argument.\n");
                    return 1;
                }

                file = argv[i + 1];
                break;
            }

            // Didn't find option
            else {                    
                printf("Ignoring unknown option: \"%s\"\n", argv[i]);
            }
        }

        else {
            file = argv[i];
            break;
        }
    }

    if (codestring.empty()) {
        if (file == NULL) {
            printf("You must input a file or code. Input \"-?\" for help.\n");
            return 1;
        }

        // Get raw data from file
        ifstream CodeFile(file, ios::binary);

        if (!CodeFile.good()) {
            printf("File \"%s\" not found.\n", file);
            return 1;
        }

        stringstream buffer;
        buffer << CodeFile.rdbuf();

        codestring = buffer.str();
    }

    if (romstart + codestring.length() > 0x10000) {
        printf("The input binary goes past the maximum memory address (0xFFFF). Input a smaller binary or set the starting address lower.\n");
        return 1;
    }

    // Load code into memory
    for (int i = 0; i < codestring.length(); i++) {
        memory[romstart + i] = codestring[i];
    }

    // Set program counter (reset vector)
    pc = 0x100 * memory[0xFFFD] + memory[0xFFFC];

    // Amount of bytes to move (0xFF means break)
    byte mvbytes = 0;

    // Instruction loop
    while (mvbytes != BRK_MOVE) {
        if (ins_print) printf("%04X %02X - ", pc, memory[pc]); // Print position and instruction before running (which might change program counter)

        byte operands[2] = {memory[pc + 1], memory[pc + 2]};
        mvbytes = instruction(memory[pc], operands);

        if (ins_print) printf("A: %02X X: %02X Y: %02X S: %02X SR/NV-BDIZC: [%d%d%d%d%d%d%d%d]\n", a, x, y, sp, sr.n, sr.v, sr._, sr.b, sr.d, sr.i, sr.z, sr.c);

        // Increment program counter
        pc += mvbytes;
    }

    if (mem_print) {
        // Output of memory once finished (rows of 8 bytes)
        for (int i = 0; i < rows && rowsize * (1 + i) + memstart <= 0x10000; i++) {
            printf("\n%04X: ", rowsize * i + memstart);

            for (int j = 0; j < rowsize; j++) {
                printf("%02X ", memory[rowsize * i + j + memstart]);
            }

            if (asc_print) cout << "| " + ascii(rowsize * i + memstart, rowsize);
        }

        printf("\n");
    }

    cout << endprint << '\n';

    return 0;
}
