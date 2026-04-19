#include "ICalculator.h"
#include "CompileRegs.h"
#include "VM.h"

InstructionCalculator::InstructionCalculator(SymbolTable& sym) : symbolTable(sym), nextReg(0) {}

int InstructionCalculator::getNewRegister() { return nextReg++; }

int InstructionCalculator::compileNode(const ASTNode* node) {
    if (!node) return -1;
    return const_cast<ASTNode*>(node)->compile(program);
}

int32_t InstructionCalculator::calculate(const ASTNode* root) {
    clear();
    compile_regs::reset();
    if (!root) return 0;
    compileNode(root);
    VirtualMachine vm(symbolTable, 64);
    return vm.execute(program);
}

const std::vector<Instruction>& InstructionCalculator::getProgram() const { return program; }

void InstructionCalculator::clear() {
    program.clear();
    nextReg = 0;
}
