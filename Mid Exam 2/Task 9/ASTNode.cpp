#include "ASTNode.h"

static int gRegCounter = 0;
static int newReg() { return gRegCounter++; }

static void emitLoadImm(std::vector<Instruction>& prog, int reg, double val) {
    int intVal = (int)val;
    if (intVal > 65535 || intVal < -32768) {
        int low  = intVal & 0xFFFF;
        int high = (intVal >> 16) & 0xFFFF;
        prog.push_back(Instruction(OpCode::LIL, LiData{(unsigned char)reg, (unsigned int)low}));
        prog.push_back(Instruction(OpCode::LIH, LiData{(unsigned char)reg, (unsigned int)high}));
    } else {
        prog.push_back(Instruction(OpCode::LI, LiData{(unsigned char)reg, (unsigned int)(int)intVal}));
    }
}

NumberNode::NumberNode(double v) { value = v; type = NodeType::NumberNode; }

int NumberNode::compile(std::vector<Instruction>& prog) const {
    int reg = newReg();
    emitLoadImm(prog, reg, value);
    return reg;
}

VariableNode::VariableNode(const std::string& n, SymbolTable& sym) : symbolTable(sym) {
    name = n; type = NodeType::VariableNode;
}

int VariableNode::compile(std::vector<Instruction>& prog) const {
    int reg = newReg();
    int idx = (int)symbolTable.getIndex(name);
    prog.push_back(Instruction(OpCode::LOAD, ArithData{(unsigned char)reg, 0, (unsigned char)idx}));
    return reg;
}

BinaryOpNode::BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r) : op(o) {
    left = std::move(l); right = std::move(r); type = NodeType::BinaryOpNode;
}

int BinaryOpNode::compile(std::vector<Instruction>& prog) const {
    int lReg = left->compile(prog);
    int rReg = right->compile(prog);
    int res  = newReg();

    switch (op) {
        case '+': prog.push_back(Instruction(OpCode::ADD, ArithData{(unsigned char)res,(unsigned char)lReg,(unsigned char)rReg})); break;
        case '-': prog.push_back(Instruction(OpCode::SUB, ArithData{(unsigned char)res,(unsigned char)lReg,(unsigned char)rReg})); break;
        case '*': prog.push_back(Instruction(OpCode::MUL, ArithData{(unsigned char)res,(unsigned char)lReg,(unsigned char)rReg})); break;
        case '/': prog.push_back(Instruction(OpCode::DIV, ArithData{(unsigned char)res,(unsigned char)lReg,(unsigned char)rReg})); break;
        case '>': {
            prog.push_back(Instruction(OpCode::CMP, ArithData{0,(unsigned char)lReg,(unsigned char)rReg}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 0}));
            int skipPc = (int)prog.size();
            prog.push_back(Instruction(OpCode::JLE, ArithData{0,0,0}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 1}));
            prog[skipPc].data = ArithData{(unsigned char)(prog.size()),0,0};
            break;
        }
        case '<': {
            prog.push_back(Instruction(OpCode::CMP, ArithData{0,(unsigned char)lReg,(unsigned char)rReg}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 0}));
            int skipPc = (int)prog.size();
            prog.push_back(Instruction(OpCode::JGE, ArithData{0,0,0}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 1}));
            prog[skipPc].data = ArithData{(unsigned char)(prog.size()),0,0};
            break;
        }
        case 'G': {
            prog.push_back(Instruction(OpCode::CMP, ArithData{0,(unsigned char)lReg,(unsigned char)rReg}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 0}));
            int skipPc = (int)prog.size();
            prog.push_back(Instruction(OpCode::JL, ArithData{0,0,0}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 1}));
            prog[skipPc].data = ArithData{(unsigned char)(prog.size()),0,0};
            break;
        }
        case 'L': {
            prog.push_back(Instruction(OpCode::CMP, ArithData{0,(unsigned char)lReg,(unsigned char)rReg}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 0}));
            int skipPc = (int)prog.size();
            prog.push_back(Instruction(OpCode::JG, ArithData{0,0,0}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 1}));
            prog[skipPc].data = ArithData{(unsigned char)(prog.size()),0,0};
            break;
        }
        case 'E': {
            prog.push_back(Instruction(OpCode::CMP, ArithData{0,(unsigned char)lReg,(unsigned char)rReg}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 0}));
            int skipPc = (int)prog.size();
            prog.push_back(Instruction(OpCode::JNE, ArithData{0,0,0}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 1}));
            prog[skipPc].data = ArithData{(unsigned char)(prog.size()),0,0};
            break;
        }
        case 'N': {
            prog.push_back(Instruction(OpCode::CMP, ArithData{0,(unsigned char)lReg,(unsigned char)rReg}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 0}));
            int skipPc = (int)prog.size();
            prog.push_back(Instruction(OpCode::JE, ArithData{0,0,0}));
            prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)res, 1}));
            prog[skipPc].data = ArithData{(unsigned char)(prog.size()),0,0};
            break;
        }
        default:
            prog.push_back(Instruction(OpCode::ADD, ArithData{(unsigned char)res,(unsigned char)lReg,(unsigned char)rReg}));
    }
    return res;
}

UnaryOpNode::UnaryOpNode(char o, std::unique_ptr<ASTNode> node) : op(o) {
    operand = std::move(node); type = NodeType::UnaryOpNode;
}

int UnaryOpNode::compile(std::vector<Instruction>& prog) const {
    int operandReg = operand->compile(prog);
    if (op == '-') {
        int zeroReg = newReg();
        int resReg  = newReg();
        prog.push_back(Instruction(OpCode::LI,  LiData{(unsigned char)zeroReg, 0}));
        prog.push_back(Instruction(OpCode::SUB, ArithData{(unsigned char)resReg,(unsigned char)zeroReg,(unsigned char)operandReg}));
        return resReg;
    }
    return operandReg;
}

AssignmentNode::AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym)
    : varName(name), symbolTable(sym) {
    expression = std::move(expr); type = NodeType::AssignmentNode;
}

int AssignmentNode::compile(std::vector<Instruction>& prog) const {
    int exprReg = expression->compile(prog);
    int idx     = (int)symbolTable.getIndex(varName);
    prog.push_back(Instruction(OpCode::STORE, ArithData{(unsigned char)idx, 0, (unsigned char)exprReg}));
    return exprReg;
}