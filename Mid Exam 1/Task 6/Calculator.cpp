#include "Calculator.h"
#include "ASTNode.h"
#include "VM.h"
#include <vector>
#include <iostream>

Calculator::Calculator(SymbolTable& st) : symbolTable(st) {}

double Calculator::calculate(std::unique_ptr<ASTNode> root) {
    if (!root) return 0.0;

    std::vector<Instruction> program;
    
    std::cout << "\n=== COMPILATION PHASE ===\n";
    std::cout << "Converting AST to Instructions:\n";
    std::cout << "------------------------------\n";
    
    int resultIdx = root->compile(program);
    
    std::cout << "\n=== GENERATED PROGRAM ===\n";
    std::cout << "Total instructions: " << program.size() << "\n";
    for (size_t i = 0; i < program.size(); i++) {
        const auto& instr = program[i];
        std::cout << "  [" << i << "] ";
        if (instr.op == OpCode::MOV) {
            std::string srcType, dstType;
            switch(instr.data.mov.srcType) {
                case OperandType::REG: srcType = "rv"; break;
                case OperandType::VAR: srcType = "var"; break;
                case OperandType::CONST: srcType = "const"; break;
            }
            switch(instr.data.mov.dstType) {
                case OperandType::REG: dstType = "rv"; break;
                case OperandType::VAR: dstType = "var"; break;
                case OperandType::CONST: dstType = "const"; break;
            }
            std::cout << "MOV " << srcType << "[" << instr.data.mov.srcIdx << "] -> " << dstType << "[" << instr.data.mov.dstIdx << "]";
        } else {
            std::string opName;
            switch(instr.op) {
                case OpCode::ADD: opName = "ADD"; break;
                case OpCode::SUB: opName = "SUB"; break;
                case OpCode::MUL: opName = "MUL"; break;
                case OpCode::DIV: opName = "DIV"; break;
                default: opName = "???"; break;
            }
            std::cout << opName << " rv[" << instr.data.arith.left << "], rv[" << instr.data.arith.right << "] -> rv[" << instr.data.arith.resIdx << "]";
        }
        std::cout << "\n";
    }
    std::cout << "=========================\n";

    std::vector<double> rv(program.size() + 10, 0.0);
    VirtualMachine vm(symbolTable);
    
    std::cout << "\n=== VM EXECUTION PHASE ===\n";
    double result = vm.execute(program, rv);
    std::cout << "=========================\n";

    return result;
}