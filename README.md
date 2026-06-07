# OOP Compiler Project

This project is a C-like compiler/interpreter toolchain written in C++.  
It parses source files, builds AST/IR, can run programs through a logical VM/interpreter path, and also supports an IR-to-RISC-V executable flow for supported subsets.

## What The Project Supports

- **Variables**: global, local, static declarations (`int`-only model).
- **Functions**: `int`/`void`, parameters, calls, return statements, recursion.
- **Control flow**: `if/else`, `while`, `for`, `do-while`, `switch/case/default`, `break`, `continue`, `goto`, labels.
- **Operators**:
  - Arithmetic: `+ - * / % // %/ **`
  - Bitwise: `& | ^ << >> ~`
  - Logical: `&& || !` and keyword forms `and or not`
  - Comparison: `== != < > <= >=`
  - Assignment: `= += -= *= /= %= ^=`
  - Ternary: `cond ? a : b`
- **Types**:
  - `int` (main runtime type)
  - `enum` constants
  - `struct` fields
  - `union` field overlay behavior
  - `class` field parsing with `public/private` access sections
- **Casts**: `(int)expr` and `static_cast<int>(expr)`.
- **Math functions**: `sin cos tan asin acos atan atan2 sqrt cbrt pow exp log ln log10 log2 log_ab ceil fmod` (+ `abs`, `sqrt`).
- **Constants**: `PI/E` and lowercase `pi/e` aliases.
- **Toolchain modules**: lexer, parser, AST, IR emission, IR file writer, IR-to-RISC-V translator, VM monitor/CPU, linker entrypoint.
- **Interpreter call stack**: function calls are tracked by linked-list stack frames.
- **Interactive RISC-V debugger** (`--debug <file.exe>`):
  - Step into (`step` / `si` / Enter), step over (`over` / `so`), step out (`out` / `su`), continue (`go` / `c`)
  - Breakpoints: `br.add <addr>`, `br.rem <addr>`, `br` (list)
  - Inspect state: `r<n>`, `m<addr>`, disassembly at PC, call depth, non-zero registers
- **Debugger hooks**:
  - `OOP_INTERP_DEBUG=1` shows interpreter function enter/exit + call stack.
  - `OOP_VM_DEBUG=1` enables VM monitor CPU trace.

## Main Folders

- `main/Compiler` - compiler, parser, VM, linker, executable.
- `main/Compiler/VM` - VM-focused folder shim (`VM.h`, `VM.cpp`).
- `tests` - automated language tests (`test_*.txt`) and test runner script.
- `tests/multifile` - extra multi-file style examples.

## Build

From project root:

```powershell
cd main\Compiler
mingw32-make
```

This builds `main/Compiler/calculator.exe`.

## Run One Program Manually

From project root:

```powershell
.\main\Compiler\calculator.exe .\tests\test_01_global_int.txt
```

## Useful CLI Modes

From `main/Compiler`:

```powershell
.\calculator.exe --help
.\calculator.exe --emit-ir <input.txt> <out.ir>
.\calculator.exe --riscv-exe <input.txt> <out.exe>
.\calculator.exe --debug <out.exe>
.\calculator.exe --link <obj1> <obj2> -o <out.exe>
```

### Debugger example

```powershell
.\calculator.exe --riscv-exe ..\..\tests\test_01_global_int.txt prog.exe
.\calculator.exe --debug prog.exe
```

Inside the debugger:

```
(dbg) br.add 10
(dbg) step
(dbg) r5
(dbg) over
(dbg) c
(dbg) q
```

## Current Limitations

- Runtime is intentionally **int-focused**; no floating-point pipeline.
- `main(int argc, char* argv[])` style signature is not part of the language grammar.
- Some advanced linker/object-format features are simplified compared to production C++ toolchains.

---

## How To Test Everything Yourself (Step-by-Step)

1. Open PowerShell in project root:
   ```powershell
   cd C:\Users\Gayane\Desktop\OOP
   ```
2. Build the compiler:
   ```powershell
   cd .\main\Compiler
   mingw32-make
   ```
3. Run the full automated test suite:
   ```powershell
   cd ..\..\tests
   .\run_all_tests.ps1
   ```
4. Optional: run one specific test manually:
   ```powershell
   ..\main\Compiler\calculator.exe .\test_30_proto_extern.txt
   ```

If the final line says all tests finished with exit code `0`, your project is passing the full suite.