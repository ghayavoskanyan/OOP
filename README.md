~ Arithmetic Expression Interpreter & Virtual Machine ~
This project is a custom-built Compiler & Register-based Virtual Machine designed to parse, compile, and execute arithmetic expressions and variable assignments.

~ Features
Hand-written Lexer & Parser: Tokenizes input and builds an Abstract Syntax Tree (AST).
State-Machine Parsing: Uses a transition matrix for robust syntax validation.
Custom Instruction Set: Compiles high-level expressions into low-level Bytecode (LOAD, ADD, MOV, STORE).
Register-based VM: Efficiently executes instructions using a dedicated Result Vector (registers) and Symbol Table (memory).

~ Project Structure
Lexer: Breaks strings into tokens (Numbers, Names, Operators).
Parser: Validates grammar and builds the AST.
ASTNodes: Handles recursive code generation (compile method).
Virtual Machine (VM): Executes the compiled instructions.
Symbol Table: Manages variable storage and mapping.

~ How it Works
Input: x = 5 + 3
Lexing: [Name:x], [Assign:=], [Num:5], [Op:+], [Num:3]
Parsing: Builds an AssignmentNode with a BinaryOpNode child.
Compilation: Generates bytecode:
LOAD_CONST r0, 5
LOAD_CONST r1, 3
ADD r2, r0, r1
STORE v[x], r2
Execution: The VM processes these steps and updates the Symbol Table.

~ How to Build and Run
To compile and execute the calculator, use:
bash
mingw32-make run

~ Tech Stack
Language: C++
Paradigm: Object-Oriented Programming (OOP), Recursive Descent Parsing.
Environment: MinGW / GCC