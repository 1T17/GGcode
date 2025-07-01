GGcode

GGcode is a custom G-code scripting language and compiler that brings programmability to CNC machining. Designed for automation, testing, and dynamic toolpath generation, it supports variables, control flow (if, for, while), expression evaluation, and runtime note/comment blocks.

Features

Lightweight compiler for custom .gg source files

Full support for variables with let

Arithmetic and logic expressions

Conditional execution: if, else, else if

Loops: while, for ... in range

Dynamic G-code generation (G1, M3, etc.)

Runtime note {} blocks with variable interpolation

Human-readable debug output

Clean, raw G-code output

Example GGcode Script

let id = 100
let x = 50
let y = 25

note {
    Program ID: [id]
    Starting cut at X[x] Y[y]
}

if x > 40 {
    G1 X[x] Y[y]
}

for i = 0..3 {
    G1 X[i*10] Y[0]
}

Building the Compiler

Requirements

GCC or Clang

make

Build

make

The compiler binary ggcode will be generated in the root folder.

Clean

make clean

Usage

Compile and run a .gg file:

./ggcode path/to/file.gg

Example output:

(EXPECT: X123 — should trigger)
N10 G1 X123

Supported Syntax Overview

Keyword

Description

let

Define variables

if

Conditional execution

else if

Chained conditional logic

else

Fallback execution

while

Loop while condition is true

for

Range-based loop

note

Insert runtime comments

G1, M3

G-code statements with parameters

Internals (Current Architecture)

Lexer – Tokenizes source

Parser – Builds AST with recursive descent

AST Nodes – Strongly typed C structs

Evaluator – Interprets expressions and conditions

Emitter – Outputs final G-code (with line numbers, formatted output)

Roadmap



License

MIT License (or update with your preferred license)

Author  https://doorbase.io

GGcode is built for control, clarity, and creativity — bringing logic to motion.

