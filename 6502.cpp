#include <fstream>
#include <sstream>

#include "ops.h"

#ifdef _WIN32
#define EXESTRING "6502.exe"
#else
#define EXESTRING "6502"
#endif

// TODO: Enable printing address (FFF9?)
int main(int argc, char** argv) {
    // Handle options and file
    bool mem_print = true;
    bool ins_print = true;

    char* file = NULL;

    int start = 0x0000;
    int rows = 8;
    int rowsize = 16;

    string codestring;

    for (int i = 1; i < argc; i++) {
        // Check for options
        if (argv[i][0] == '-') {
            if (argv[i] == string("-?")) {
                printf(
                    "6502 HELP\n"
                    "  Usage:\n"
                    "    %s [-options | --flags] [-o code] [-f] {file}\n"
                    "\n"
                    "  Options:\n"
                    "    -?        Display help and stop execution (this message).\n"
                    "    -mr {x}   Select the number of rows to print out when displaying memory (default 8).\n"
                    "    -ml {x}   Select the length of rows to print out when displaying memory (default 16).\n"
                    "    -ms {x}   Select the starting value of memory to print rows from. Prefix with \"0x\" for hex input (default 0).\n"
                    "    --m       Disable the printing of memory once hitting a break (default true).\n"
                    "    --i       Disable the printing of instructions while executing (default true).\n"
                    "    -o {x}    Directly input assembled machine code as string and ignore rest of input.\n"
                    "    -f {x}    Explicitly select the input file to be executed and ignore rest of input (default last arg).\n",
                    EXESTRING
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

    if (codestring.empty()) {
        if (file == NULL) {
            printf("You must input a file or code. Input \"-?\" for help.\n");
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

        codestring = buffer.str();
    }
    
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
        mvbytes = instruction(memory[pc], new (unsigned char[2]) {memory[pc + 1], memory[pc + 2]});

        if (ins_print) printf("%04X %02X - A: %02X X: %02X Y: %02X SR/NV-BDIZC: [%d%d%d%d%d%d%d%d]\n", pc, memory[pc], a, x, y, sr.n, sr.v, sr._, sr.b, sr.d, sr.i, sr.z, sr.c);

        // Increment program counter
        pc += mvbytes;
    }

    if (mem_print) {
        // Output of memory once finished (rows of 8 bytes)
        for (int i = 0; i < rows && rowsize * (1 + i) + start <= 0x10000; i++) {
            printf("\n%04X: ", rowsize * i + start);

            for (int j = 0; j < rowsize; j++) {
                printf("%02X ", memory[rowsize * i + j + start]);
            }
        }

        printf("\n");
    }

    return 0;
}
