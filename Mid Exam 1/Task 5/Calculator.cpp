#include "Calculator.h"
#include "ASTNode.h"
#include "VM.h"
#include <vector>
#include <iostream>

Calculator::Calculator(SymbolTable& st) : symbolTable(st) {}

double Calculator::calculate(std::unique_ptr<ASTNode> root) {
    if (!root) return 0.0;

    std::vector<Instruction> program;
    int resultIdx = root->compile(program);

    std::vector<double> rv(program.size() + 10, 0.0);
    VirtualMachine vm(symbolTable);
    
    std::cout << "\n--- VM Iterative Execution ---" << "\n";
    double result = vm.execute(program, rv);
    std::cout << "--- End ---" << std::endl;

    return result;
}