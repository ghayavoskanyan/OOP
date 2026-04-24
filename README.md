# OOP Compiler Project

This project is a C-like compiler/interpreter toolchain written in C++.  
It parses source files, builds AST/IR, can run programs through a logical VM/interpreter path, and also supports an IR-to-RISC-V executable flow for supported subsets.

## What The Project Supports

- **Variables**: global, local, static declarations (`int`-only model).
- **Functions**: `int`/`void`, parameters, calls, return statements, recursion.
- **Control flow**: `if/else`, `while`, `for`, `do-while`, `switch/case/default`, `break`, `continue`, `goto`, labels.
- **Types**:
  - `int` (main runtime type)
  - `enum` constants
  - `struct` fields
  - `union` field overlay behavior
  - `class` field parsing with `public/private` access sections
- **Casts**: `(int)expr` and `static_cast<int>(expr)`.
- **Toolchain modules**: lexer, parser, AST, IR emission, IR file writer, IR-to-RISC-V translator, VM monitor/CPU, linker entrypoint.

## Main Folders

- `main/Compiler` - compiler, parser, VM, linker, executable.
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
.\calculator.exe --link <obj1> <obj2> -o <out.exe>
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