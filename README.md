# ez80dis
This is a simple disassembler for the Zilog eZ80 instruction set.

## Building
Use `make` to build ez80dis.

Alternatively, as `ez80dis` has no dependencies other than the C
standard library, the equivalent can be written for whatever
compiler
`cc -o ez80dis src/*.c`
or for Microsoft Visual C++
`cl src/*.c /Fe:ez80dis.exe`

## Usage
Usage information can be obtained by typing
    
```
$ ez80dis -help
USAGE: ez80dis [options] <input>

OPTIONS:
-help      Print this message
-A         Assume ADL mode
-org <a>   Set the base address of the file
-sym <f>   Add specified symbol file
-start <s> Specify the position at which to start disassembling
-end <a>   Specify the point at which to stop disassembling
-wno       Supress warnings
-o <f>     Specify the output file

```
    
`ez80dis` takes a single binary file to disassemble, and the point at
which to start disassembly is given by the `-start` option and
at which to finish is given by the `-end` option. Otherwise, it will
attempt to disassemble the entirety of the file.

This feature allows the disassembly, for example, of code sections of
an executable, while data sections can be displayed with any simple
    hexdump utility, such as xxd.
    
By default it outputs the listings to stdout, but this can be redirected
to an output file through the `-o` option.
    
The `-org` option tells the disassembler what is the base address of the
file, not the point at which disassembly starts.

## License
[GPLv3](https://choosealicense.com/licenses/gpl-3.0/)