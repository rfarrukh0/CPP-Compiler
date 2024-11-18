## WLP4 Compiler
This project is a WLP4 (a subset of C++) compiler that tokenizes, verifies syntax and semantics, and translates WLP4 code into MIPS Assembly. It is designed to demonstrate an in-depth understanding of compiler design, including lexical analysis, parsing, syntax tree generation, and code generation.

## Features
Lexical analysis and token generation.
Syntax and semantic verification using a custom parser.
Translation of valid WLP4 code into MIPS Assembly.
Handling of complex language constructs, including expressions, control structures, and function calls.

## Installation
Clone the repository to your local machine:
```
git clone https://github.com/rfarrukh0/CPP-Compiler
cd CPP-Compiler
```

Make sure you have g++ installed on your system

```
Compile the Compiler
g++ wlp4gen.cc -o wlp4gen
```

## Usage
After installation, compile and run the compiler with your WLP4 code:
```
Compile your WLP4 program:
./wlp4gen < input.wlp4 > output.mips

This will generate a MIPS assembly file (output.mips) from your WLP4 source code (input.wlp4).
```

## Quick Example
The Wlp4 program:
```
int wain(int a, int b) {
    return a + b;
}
```

Gets translated into:
```
.import print
.import init
.import new
.import delete
procedurewain:
lis $4
.word 4
lis $11
.word 1
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
sub $29, $30, $4
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word init
lis $2
.word 0
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, 8($29)
add $5, $0, $3
sw $5, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
add $30, $30, $4
add $30, $30, $4
jr $31
```
