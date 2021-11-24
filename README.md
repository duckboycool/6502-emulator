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
    -sa {x}   Set the starting address for the ROM. Prefix with "0x" for hex input (default 0x0000).
    -mr {x}   Set the number of rows to print out when displaying memory (default 8).
    -ml {x}   Set the length of rows to print out when displaying memory (default 16).
    -ms {x}   Set the starting value of memory to print rows from. Prefix with "0x" for hex input (default 0).
    --m       Disable the printing of memory once hitting a break (default true).
    --a       Disable ascii representation of printed memory (default true).
    --i       Disable the printing of instructions while executing (default true).
    --p       Enable the printing address (will only print at end if --m or --i) (default false).
    --b       Enable continuation on BRK, fetching position from IRQ vector (default false).
    -pa {x}   Set value of printing address. Prefix with "0x" for hex input (default 0xFFF9).
    -mf {x}   Set maximum operation frequency for emulator. Suffix with "k" or "m" for thousands or millions (default none).
    -o {x}    Directly input assembled machine code as string and ignore rest of input.
    -f {x}    Explicitly select the input file to be executed and ignore rest of input (default last arg).
```

You will need to assemble your program from 6502 assembly before inputting it into `6502.exe`.

There are example programs in the `ExamplePrograms` folder. The examples come both in 6502 assembly source (`.65s` files) and assembled machine code (`.out` files).