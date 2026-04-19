#include <iostream>
#include <memory>
#include <string>
#include "CompileRegs.h"
#include "Lexer.h"
#include "Manager.h"
#include "StatementParser.h"
#include "StmtInterpreter.h"
#include "VM.h"
#include "Linker.h"

static void printHelp() {
    std::cout << "\n";
    std::cout << "  OOP Compiler pipeline (int-only IR + optional RV32IM executable)\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    std::cout << "  calculator.exe <file.txt>              Run source (logical VM)\n";
    std::cout << "  calculator.exe --riscv-exe <in> <out>  Compile to .exe, run RISC-V VM\n";
    std::cout << "  calculator.exe --emit-ir <in> <out.ir> Write logical IR file\n";
    std::cout << "  calculator.exe --link <o1> <o2> ... -o <out.exe>  Link object/exe stubs\n";
    std::cout << "  Interactive: + - * /, comparisons, if/else, while, for, print(expr)\n";
    std::cout << "  Commands: run <file>, vars, quit\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
}

int main(int argc, char* argv[]) {
    Manager manager;

    if (argc >= 2) {
        std::string a1 = argv[1];
        if (a1 == "--riscv-exe" && argc >= 4) {
            std::string err;
            if (!manager.compileFileToExe(argv[2], argv[3])) {
                std::cerr << "Compile failed.\n";
                return 1;
            }
            if (!manager.runRiscvExe(argv[3], err)) {
                std::cerr << "VM: " << err << "\n";
                return 1;
            }
            return 0;
        }
        if (a1 == "--emit-ir" && argc >= 4) {
            if (!manager.compileFileToIr(argv[2], argv[3])) {
                std::cerr << "IR emit failed.\n";
                return 1;
            }
            std::cout << "Wrote IR: " << argv[3] << "\n";
            return 0;
        }
        if (a1 == "--link") {
            std::vector<std::string> objs;
            std::string out;
            for (int i = 2; i < argc; ++i) {
                std::string a = argv[i];
                if (a == "-o" && i + 1 < argc) {
                    out = argv[i + 1];
                    ++i;
                } else {
                    objs.push_back(a);
                }
            }
            if (out.empty() || objs.empty()) {
                std::cerr << "Usage: --link <obj> ... -o <out.exe>\n";
                return 1;
            }
            if (!linker::linkObjectFiles(objs, out)) {
                std::cerr << "Link failed.\n";
                return 1;
            }
            std::cout << "Wrote " << out << "\n";
            return 0;
        }

        std::string filepath = argv[1];
        try {
            manager.runFile(filepath);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    printHelp();

    std::string line;
    std::string multilineBuffer;
    int braceDepth = 0;

    while (true) {
        if (braceDepth > 0) {
            std::cout << "... ";
        } else {
            std::cout << ">>> ";
        }

        if (!std::getline(std::cin, line)) break;

        if (line == "quit" || line == "exit") break;
        if (line == "help") {
            printHelp();
            continue;
        }
        if (line.empty()) continue;

        if (line.substr(0, 4) == "run ") {
            std::string filepath = line.substr(4);
            while (!filepath.empty() && filepath.front() == ' ') filepath.erase(filepath.begin());
            try {
                manager.runFile(filepath);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
            continue;
        }

        if (line == "vars") {
            manager.printAllVariables();
            continue;
        }

        for (char c : line) {
            if (c == '{') braceDepth++;
            else if (c == '}') braceDepth--;
        }
        multilineBuffer += line + " ";

        if (braceDepth > 0) continue;

        std::string input = multilineBuffer;
        multilineBuffer.clear();
        braceDepth = 0;

        try {
            std::istringstream iss(input);
            Lexer lexer(iss);
            StatementParser parser(lexer, manager.getSymbolTable());

            auto stmt = parser.parse();
            if (!stmt) continue;

            int32_t result = 0;
            if (programNeedsInterpreter(stmt.get())) {
                StmtInterpreter interp(manager.getSymbolTable());
                result = interp.run(stmt.get());
            } else {
                std::vector<Instruction> prog;
                compile_regs::reset();
                stmt->compile(prog);
                VirtualMachine vm(manager.getSymbolTable());
                result = vm.execute(prog);
            }

            bool hasAssignment = (input.find('=') != std::string::npos && input.find("==") == std::string::npos);
            bool hasPrint = (input.find("print") != std::string::npos);
            bool hasControl = (input.find("if") != std::string::npos || input.find("while") != std::string::npos ||
                               input.find("for") != std::string::npos || input.find("switch") != std::string::npos);

            if (!hasPrint && !hasControl) {
                std::cout << "= " << result << "\n";
            }

            if (hasAssignment && !hasPrint && !hasControl) {
                size_t eqPos = input.find('=');
                std::string varName = input.substr(0, eqPos);
                size_t first = varName.find_first_not_of(" \t");
                if (first != std::string::npos) {
                    varName = varName.substr(first);
                    size_t last = varName.find_last_not_of(" \t");
                    if (last != std::string::npos) varName = varName.substr(0, last + 1);
                }
                int32_t val = 0;
                if (manager.getVariable(varName, val)) {
                    std::cout << varName << " = " << val << "\n";
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    std::cout << "Bye!\n";
    return 0;
}
