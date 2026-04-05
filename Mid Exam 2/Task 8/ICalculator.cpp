/*ICalculator.cpp*/ #include "ICalculator.h"
#include "Traverser.h"
#include <iostream>
#include <unordered_map>

InstructionCalculator::InstructionCalculator(SymbolTable& sym) : symbolTable(sym), nextReg(0) {}

int InstructionCalculator::getNewRegister() { return nextReg++; }

int InstructionCalculator::compileNode(const ASTNode* node) {
    if (!node) return -1;
    static std::unordered_map<const ASTNode*, int> nodeToReg;
    switch (node->type) {
        case NodeType::NumberNode: {
            auto* num = static_cast<const NumberNode*>(node);
            int reg = getNewRegister();
            int intVal = (int)num->getValue();
            if (intVal > 65535) {
                int low = intVal & 0xFFFF, high = (intVal >> 16) & 0xFFFF;
                program.push_back(Instruction(OpCode::LIL, LiData{(unsigned char)reg, (unsigned int)low}));
                program.push_back(Instruction(OpCode::LIH, LiData{(unsigned char)reg, (unsigned int)high}));
            } else {
                program.push_back(Instruction(OpCode::LI, LiData{(unsigned char)reg, (unsigned int)intVal}));
            }
            nodeToReg[node] = reg; return reg;
        }
        case NodeType::VariableNode: {
            auto* var = static_cast<const VariableNode*>(node);
            int reg = getNewRegister();
            int varIdx = symbolTable.getIndex(var->getName());
            program.push_back(Instruction(OpCode::LOAD, ArithData{(unsigned char)reg, 0, (unsigned char)varIdx}));
            nodeToReg[node] = reg; return reg;
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
                case '>': code = OpCode::CMP; break;
                default: code = OpCode::ADD;
            }
            program.push_back(Instruction(code, ArithData{(unsigned char)resReg, (unsigned char)leftReg, (unsigned char)rightReg}));
            nodeToReg[node] = resReg; return resReg;
        }
        case NodeType::UnaryOpNode: {
            auto* unary = static_cast<const UnaryOpNode*>(node);
            if (unary->getOp() == '-') {
                int operandReg = compileNode(unary->getOperand().get());
                int zeroReg = getNewRegister(), resReg = getNewRegister();
                program.push_back(Instruction(OpCode::LI, LiData{(unsigned char)zeroReg, 0}));
                program.push_back(Instruction(OpCode::SUB, ArithData{(unsigned char)resReg, (unsigned char)zeroReg, (unsigned char)operandReg}));
                nodeToReg[node] = resReg; return resReg;
            }
            return compileNode(unary->getOperand().get());
        }
        case NodeType::AssignmentNode: {
            auto* assign = static_cast<const AssignmentNode*>(node);
            int exprReg = compileNode(assign->getExpression().get());
            int varIdx = symbolTable.getIndex(assign->getVarName());
            program.push_back(Instruction(OpCode::STORE, ArithData{(unsigned char)varIdx, 0, (unsigned char)exprReg}));
            nodeToReg[node] = exprReg; return exprReg;
        }
        default: return -1;
    }
}

double InstructionCalculator::calculate(const ASTNode* root) {
    clear(); if (!root) return 0.0;
    compileNode(root);
    VirtualMachine vm(symbolTable, 32);
    return vm.execute(program);
}

const std::vector<Instruction>& InstructionCalculator::getProgram() const { return program; }
void InstructionCalculator::clear() { program.clear(); nextReg = 0; }