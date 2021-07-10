# Manosim
A ”Mano Machine” simulator based on 8085 Architecture written in C++ with heavy usage of the Standard Template Library (STL). For convinience purposes, native C library is used for graphics and only a small instruction set is supported. A pre-compiled binary has been provided, and the source code can be compiled with the help of Code Blocks, following [this thread.](https://stackoverflow.com/questions/20313534/how-to-use-graphics-h-in-codeblocks)

# Instructions
Write instructions as given in _add.as_ file. The program should terminal with HLT in the middle and the origin is set by ORG. Pre-halt phase consists of instructions and Post-halt phase consists of variable declarations.

Supported Instructions: LDA, STA, ADD, SUB, MUL, DIV, XOR, OR, AND
