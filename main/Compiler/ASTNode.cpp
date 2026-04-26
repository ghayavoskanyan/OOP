#include "ASTNode.h"
#include "CompileRegs.h"
#include "IrEmit.h"

NumberNode::NumberNode(int32_t v) {
    value = v;
    type = NodeType::NumberNode;
}

int NumberNode::compile(std::vector<Instruction>& prog) const {
    int reg = compile_regs::newReg();
    emitLoadImm(prog, reg, value);
    return reg;
}

VariableNode::VariableNode(const std::string& n, SymbolTable& sym) : symbolTable(sym) {
    name = n; type = NodeType::VariableNode;
}

int VariableNode::compile(std::vector<Instruction>& prog) const {
    int reg = compile_regs::newReg();
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
    int res  = compile_regs::newReg();

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
        int zeroReg = compile_regs::newReg();
        int resReg  = compile_regs::newReg();
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
    prog.push_back(Instruction(OpCode::STORE, ArithData{static_cast<uint32_t>(idx), 0, static_cast<uint32_t>(exprReg)}));
    return exprReg;
}

CastNode::CastNode(std::unique_ptr<ASTNode> e) : inner(std::move(e)) {
    type = NodeType::CastNode;
}

int CastNode::compile(std::vector<Instruction>& prog) const {
    if (!inner) return 0;
    return inner->compile(prog);
}

CallNode::CallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> a, SymbolTable& sym)
    : funcName(std::move(name)), args(std::move(a)), symbolTable(sym) {
    type = NodeType::CallNode;
}

int CallNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("Function calls are executed by the statement interpreter, not the stack VM.");
}

TernaryNode::TernaryNode(std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> y, std::unique_ptr<ASTNode> n)
    : cond_(std::move(c)), yes_(std::move(y)), no_(std::move(n)) {
    type = NodeType::TernaryNode;
}

int TernaryNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("Ternary operator is handled by the statement interpreter");
}