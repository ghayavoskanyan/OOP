```markdown
# Compiler Project – C‑like Language to RISC‑V / VM

This project implements a compiler for a C‑like input language, targeting a custom 32‑bit RISC‑V instruction set and a virtual machine.  
It supports functions, variables (global/local/static), control flow (`if`, `while`, `for`, `do‑while`, `switch`, `goto`), `struct`/`union`/`enum`, and basic type casts.

## 📁 Project Structure (current)

```
.
├── ASTNode.cpp/.h          # Abstract Syntax Tree nodes
├── CompileRegs.cpp/.h      # Register allocation / compilation utilities
├── ExeImage.cpp/.h         # Executable image format (header, sections)
├── ExprParser.cpp/.h       # Expression parsing (used by main parser)
├── ICalculator.cpp/.h      # Intermediate calculation / IR generation
├── IrEmit.h                # IR emission helpers
├── IrFile.cpp/.h           # IR file representation
├── IrToRiscv.cpp/.h        # IR → RISC‑V code generator
├── Lexer.cpp/.h            # Lexical analyzer
├── Linker.cpp/.h           # Links multiple object files
├── main.cpp                # Compiler driver
├── Manager.cpp/.h          # Manages compilation units, symbol tables
├── Makefile
├── Parser.cpp/.h           # Top‑down parser for statements
├── RiscvCpu.cpp/.h         # RISC‑V CPU emulator (inside VM)
├── RiscvISA.h              # RISC‑V instruction definitions
├── StatementNode.cpp/.h    # Statement AST nodes
├── StatementParser.cpp/.h  # Parses statements (if, while, etc.)
├── StmtInterpreter.cpp/.h  # Interprets statements (alternative to codegen)
├── SymbolTable.cpp/.h      # Symbol table with scopes
├── Token.cpp/.h            # Token representation
├── Traverser.cpp/.h        # AST traverser (for analysis / optimisation)
├── VM.cpp/.h               # Virtual machine (executes RISC‑V code)
├── VmMonitor.cpp/.h        # VM monitor (debugger, step, breakpoints)
└── example.txt             # Example input program
```

## 🚀 Building and Running

### The compiler will:
- Lex & parse the source
- Build an AST
- Generate IR (intermediate representation)
- Translate IR to RISC‑V assembly / binary
- Produce an executable image (`ExeImage`)

## 📝 Input Language Features

### Variables
- Global, local, static (inside and outside functions)
- Only `int` type (no `double`, `char`)

### Functions
- `int name(int param1, int param2) { ... }`
- `void` functions (no return value)
- `return expr;`

### Control Flow
- `if (cond) { ... } else if (cond) { ... } else { ... }`
- `while (cond) { ... }`
- `for (init; cond; step) { ... }`
- `do { ... } while (cond);`
- `switch (expr) { case c1: ... break; default: ... }`
- `goto label;` and `label:`

### Data Types
- `struct`, `union`, `class` (with public/private members)
- `enum` (named constants)

### Type Casts
- `(int)expr` – C‑style cast
- `static_cast<int>(expr)` (preferred)

## ⚙️ Compiler Phases (as implemented)

1. **Lexer** (`Lexer.cpp`) → token stream
2. **Parser** (`Parser.cpp`, `StatementParser.cpp`, `ExprParser.cpp`) → AST
3. **Semantic analysis** (`SymbolTable.cpp`, `Traverser.cpp`) – scope checking, variable resolution
4. **IR generation** (`ICalculator.cpp`, `IrFile.cpp`, `IrEmit.h`) → three‑address code / custom IR
5. **IR optimisation** (optional, via `Traverser`)
6. **Code generation** (`IrToRiscv.cpp`, `CompileRegs.cpp`) → RISC‑V instructions (defined in `RiscvISA.h`)
7. **Object / Executable image** (`ExeImage.cpp`) – combines code, data, bss into a loadable format
8. **Linking** (`Linker.cpp`) – merges multiple object files
9. **Loading** (`ExeImage` → `VM.cpp`) – loads into VM memory
10. **Execution** (`RiscvCpu.cpp` inside `VM.cpp`) – interprets RISC‑V instructions
11. **Debugging** (`VmMonitor.cpp`) – step, breakpoints, register/memory inspection

## 🧪 Testing

You can test the compiler like this:
```bash
cd main/Compiler
mingw32-make clean
mingw32-make 
mingw32-make run
```

## 📌 Known Limitations

- Only `int` type – no floating point, no char
- No lambda functions
- No dynamic memory allocation (malloc/free) – could be added via syscalls
- The VM is a pure interpreter; JIT not implemented

---

**Author:** Gayane Voskanyan
**Project:** Compiler for C‑like language → RISC‑V / VM