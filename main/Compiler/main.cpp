#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include "Manager.h"
#include "Lexer.h"
#include "StatementParser.h"
#include "VM.h"

static void printHelp() {
    std::cout << "\n";
    std::cout << "  Expression Calculator\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    std::cout << "  Supported operations : +  -  *  /\n";
    std::cout << "  Comparisons          : >  <  >=  <=  ==  !=\n";
    std::cout << "  Control flow         : if / else / while / for\n";
    std::cout << "  Output               : print(expr)\n";
    std::cout << "  File execution       : run <filename>\n";
    std::cout << "  Show variables       : vars\n";
    std::cout << "  Exit                 : quit / exit\n";
    std::cout << "\n";
    std::cout << "  Examples:\n";
    std::cout << "    x = 5 + 3\n";
    std::cout << "    y = x * 2\n";
    std::cout << "    print(y)\n";
    std::cout << "    if (x > 3) { x = x + 1; }\n";
    std::cout << "    while (x > 0) { x = x - 1; print(x); }\n";
    std::cout << "    for (i = 0; i < 5; i = i + 1) { print(i); }\n";
    std::cout << "    run program.txt\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n";
}

int main(int argc, char* argv[]) {
    Manager manager;

    if (argc >= 2) {
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
        if (line == "help") { printHelp(); continue; }
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
            // Create a fresh lexer for this input line
            std::istringstream iss(input);
            Lexer lexer(iss);
            StatementParser parser(lexer, manager.getSymbolTable());

            auto stmt = parser.parse();
            if (!stmt) continue;

            std::vector<Instruction> prog;
            stmt->compile(prog);
            VirtualMachine vm(manager.getSymbolTable());
            double result = vm.execute(prog);

            bool hasAssignment = (input.find('=') != std::string::npos &&
                                  input.find("==") == std::string::npos);
            bool hasPrint      = (input.find("print") != std::string::npos);
            bool hasControl    = (input.find("if") != std::string::npos ||
                                  input.find("while") != std::string::npos ||
                                  input.find("for") != std::string::npos);

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
                    if (last != std::string::npos)
                        varName = varName.substr(0, last + 1);
                }
                double val;
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