# 6502-emulator

A C++ emulator of the 6502 microprocessor, known for its use in the NES, Apple II, Atari 2600, Commodore 64, and many more influential systems.

## Use
To input code to be executed, you give a command line argument of the filepath to an assembled binary file with either the `-f` argument or as the last argument given. You can also input the assembled machine code directly as a string with the `-o` option.

Here is the list of options.
```
6502 HELP
  Usage:
    6502.exe [-options | --flags] [-o code] [-f] {file}

  Options:
    -?        Display help and stop execution (this message).
    -mr {x}   Select the number of rows to print out when displaying memory (default 8).
    -ml {x}   Select the length of rows to print out when displaying memory (default 16).
    -ms {x}   Select the starting value of memory to print rows from. Prefix with "0x" for hex input (default 0).
    --m       Disable the printing of memory once hitting a break (default true).
    --a       Disable ascii representation of printed memory (default true).
    --i       Disable the printing of instructions while executing (default true).
    -o {x}    Directly input assembled machine code as string and ignore rest of input.
    -f {x}    Explicitly select the input file to be executed and ignore rest of input (default last arg).
```

You will need to assemble your program from 6502 assembly before inputting it into `6502.exe`.