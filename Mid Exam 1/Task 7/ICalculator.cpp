/*ICalculator.cpp*/ #include "ICalculator.h"
#include "Traverser.h"
#include <iostream>
#include <unordered_map>

InstructionCalculator::InstructionCalculator(SymbolTable& sym) 
    : symbolTable(sym), nextReg(0) {}

int InstructionCalculator::getNewRegister() {
    return nextReg++;
}

int InstructionCalculator::compileNode(const ASTNode* node) {
    if (!node) return -1;
    
    static std::unordered_map<const ASTNode*, int> nodeToReg;
    
    switch (node->type) {
        case NodeType::NumberNode: {
            auto* num = static_cast<const NumberNode*>(node);
            int reg = getNewRegister();
            int intVal = (int)num->getValue();
            if (intVal > 65535) {
                int low = intVal & 0xFFFF;
                int high = (intVal >> 16) & 0xFFFF;
                program.push_back(Instruction(OpCode::LIL, LiData{(unsigned char)reg, (unsigned int)low}));
                program.push_back(Instruction(OpCode::LIH, LiData{(unsigned char)reg, (unsigned int)high}));
            } else {
                program.push_back(Instruction(OpCode::LI, LiData{(unsigned char)reg, (unsigned int)intVal}));
            }
            nodeToReg[node] = reg;
            return reg;
        }
        
        case NodeType::VariableNode: {
            auto* var = static_cast<const VariableNode*>(node);
            int reg = getNewRegister();
            int varIdx = symbolTable.getIndex(var->getName());
            program.push_back(Instruction(OpCode::LOAD, ArithData{(unsigned char)reg, 0, (unsigned char)varIdx}));
            nodeToReg[node] = reg;
            return reg;
        }
        
        case NodeType::BinaryOpNode: {
            auto* bin = static_cast<const BinaryOpNode*>(node);
            int leftReg = compileNode(bin->getLeft().get());
            int rightReg = compileNode(bin->getRight().get());
            int resReg = getNewRegister();
            
            OpCode code;
            switch (bin->getOp()) {
                case '+': code = OpCode::ADD; break;
                case '-': code = OpCode::SUB; break;
                case '*': code = OpCode::MUL; break;
                case '/': code = OpCode::DIV; break;
                default: code = OpCode::ADD;
            }
            
            program.push_back(Instruction(code, ArithData{(unsigned char)resReg, (unsigned char)leftReg, (unsigned char)rightReg}));
            nodeToReg[node] = resReg;
            return resReg;
        }
        
        case NodeType::UnaryOpNode: {
            auto* unary = static_cast<const UnaryOpNode*>(node);
            if (unary->getOp() == '-') {
                int operandReg = compileNode(unary->getOperand().get());
                int zeroReg = getNewRegister();
                int resReg = getNewRegister();
                program.push_back(Instruction(OpCode::LI, LiData{(unsigned char)zeroReg, 0}));
                program.push_back(Instruction(OpCode::SUB, ArithData{(unsigned char)resReg, (unsigned char)zeroReg, (unsigned char)operandReg}));
                nodeToReg[node] = resReg;
                return resReg;
            }
            return compileNode(unary->getOperand().get());
        }
        
        case NodeType::AssignmentNode: {
            auto* assign = static_cast<const AssignmentNode*>(node);
            int exprReg = compileNode(assign->getExpression().get());
            int varIdx = symbolTable.getIndex(assign->getVarName());
            program.push_back(Instruction(OpCode::LOAD, ArithData{(unsigned char)varIdx, 0, (unsigned char)exprReg}));
            nodeToReg[node] = exprReg;
            return exprReg;
        }
    }
    
    return -1;
}

double InstructionCalculator::calculate(const ASTNode* root) {
    clear();
    if (!root) return 0.0;
    
    std::cout << "\n=== COMPILATION PHASE ===\n";
    std::cout << "Converting AST to Instructions (Iterative, no recursion):\n";
    std::cout << "------------------------------\n";
    
    compileNode(root);
    
    std::cout << "\n=== GENERATED PROGRAM ===\n";
    std::cout << "Total instructions: " << program.size() << "\n";
    for (size_t i = 0; i < program.size(); i++) {
        const auto& instr = program[i];
        std::cout << "  [" << i << "] ";
        switch (instr.opcode) {
            case OpCode::LI: {
                auto& d = std::get<LiData>(instr.data);
                std::cout << "LI r" << (int)d.dest << ", " << d.value;
                break;
            }
            case OpCode::LOAD: {
                auto& d = std::get<ArithData>(instr.data);
                if (d.dest < symbolTable.getValuesVector().size()) {
                    std::cout << "STORE var[" << (int)d.dest << "], r" << (int)d.right;
                } else {
                    std::cout << "LOAD r" << (int)d.dest << ", [v" << (int)d.right << "]";
                }
                break;
            }
            case OpCode::ADD: {
                auto& d = std::get<ArithData>(instr.data);
                std::cout << "ADD r" << (int)d.dest << ", r" << (int)d.left << ", r" << (int)d.right;
                break;
            }
            case OpCode::SUB: {
                auto& d = std::get<ArithData>(instr.data);
                std::cout << "SUB r" << (int)d.dest << ", r" << (int)d.left << ", r" << (int)d.right;
                break;
            }
            case OpCode::MUL: {
                auto& d = std::get<ArithData>(instr.data);
                std::cout << "MUL r" << (int)d.dest << ", r" << (int)d.left << ", r" << (int)d.right;
                break;
            }
            case OpCode::DIV: {
                auto& d = std::get<ArithData>(instr.data);
                std::cout << "DIV r" << (int)d.dest << ", r" << (int)d.left << ", r" << (int)d.right;
                break;
            }
            case OpCode::LIL: {
                auto& d = std::get<LiData>(instr.data);
                std::cout << "LIL r" << (int)d.dest << ", " << (d.value & 0xFFFF);
                break;
            }
            case OpCode::LIH: {
                auto& d = std::get<LiData>(instr.data);
                std::cout << "LIH r" << (int)d.dest << ", " << ((d.value >> 16) & 0xFFFF);
                break;
            }
            default: break;
        }
        std::cout << "\n";
    }
    std::cout << "=========================\n";
    
    VirtualMachine vm(symbolTable, 32);
    
    std::cout << "\n=== VM EXECUTION PHASE ===\n";
    double result = vm.execute(program);
    std::cout << "=========================\n";
    
    return result;
}

const std::vector<Instruction>& InstructionCalculator::getProgram() const {
    return program;
}

void InstructionCalculator::clear() {
    program.clear();
    nextReg = 0;
}