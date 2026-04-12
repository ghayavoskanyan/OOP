#include "StatementNode.h"
#include <iostream>

ExprStatement::ExprStatement(std::unique_ptr<ASTNode> e) {
    expr = std::move(e); type = StatementType::ExprStatement;
}
int ExprStatement::compile(std::vector<Instruction>& prog) const {
    if (expr) return expr->compile(prog);
    return 0;
}

PrintNode::PrintNode(std::unique_ptr<ASTNode> e, SymbolTable& sym) : symbolTable(sym) {
    expr = std::move(e); type = StatementType::PrintNode;
}
int PrintNode::compile(std::vector<Instruction>& prog) const {
    int reg = expr->compile(prog);
    prog.push_back(Instruction(OpCode::PRINT, ArithData{(unsigned char)reg, 0, 0}));
    return reg;
}

IfNode::IfNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> thenStmt,
               std::unique_ptr<StatementNode> elseStmt) {
    condition = std::move(cond);
    thenBody  = std::move(thenStmt);
    elseBody  = std::move(elseStmt);
    type = StatementType::IfNode;
}

int IfNode::compile(std::vector<Instruction>& prog) const {
    int condReg = condition->compile(prog);

    prog.push_back(Instruction(OpCode::LI, LiData{0, 0}));
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, (unsigned char)condReg, 0}));

    size_t jeIdx = prog.size();
    prog.push_back(Instruction(OpCode::JE, ArithData{0, 0, 0}));

    if (thenBody) thenBody->compile(prog);

    if (elseBody) {
        size_t jmpIdx = prog.size();
        prog.push_back(Instruction(OpCode::JMP, ArithData{0, 0, 0}));

        size_t elseStart = prog.size();
        prog[jeIdx].data = ArithData{(unsigned char)elseStart, 0, 0};

        elseBody->compile(prog);

        size_t afterElse = prog.size();
        prog[jmpIdx].data = ArithData{(unsigned char)afterElse, 0, 0};
    } else {
        size_t afterThen = prog.size();
        prog[jeIdx].data = ArithData{(unsigned char)afterThen, 0, 0};
    }
    return 0;
}

WhileNode::WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> stmt) {
    condition = std::move(cond); body = std::move(stmt); type = StatementType::WhileNode;
}

int WhileNode::compile(std::vector<Instruction>& prog) const {
    size_t startLabel = prog.size();

    int condReg = condition->compile(prog);

    prog.push_back(Instruction(OpCode::LI, LiData{0, 0}));
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, (unsigned char)condReg, 0}));

    size_t jeIdx = prog.size();
    prog.push_back(Instruction(OpCode::JE, ArithData{0, 0, 0}));

    if (body) body->compile(prog);

    prog.push_back(Instruction(OpCode::JMP, ArithData{(unsigned char)startLabel, 0, 0}));

    size_t afterLoop = prog.size();
    prog[jeIdx].data = ArithData{(unsigned char)afterLoop, 0, 0};

    return 0;
}

ForNode::ForNode(std::unique_ptr<ASTNode> i, std::unique_ptr<ASTNode> c,
                 std::unique_ptr<ASTNode> u, std::unique_ptr<StatementNode> b) {
    init = std::move(i); condition = std::move(c);
    update = std::move(u); body = std::move(b);
    type = StatementType::ForNode;
}

int ForNode::compile(std::vector<Instruction>& prog) const {
    if (init) init->compile(prog);

    size_t startLabel = prog.size();

    int condReg = 0;
    if (condition) {
        condReg = condition->compile(prog);
    } else {
        prog.push_back(Instruction(OpCode::LI, LiData{0, 1}));
        condReg = (int)prog.size() - 1;
    }

    prog.push_back(Instruction(OpCode::LI, LiData{1, 0}));
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, (unsigned char)condReg, 1}));

    size_t jeIdx = prog.size();
    prog.push_back(Instruction(OpCode::JE, ArithData{0, 0, 0}));

    if (body) body->compile(prog);

    if (update) update->compile(prog);

    prog.push_back(Instruction(OpCode::JMP, ArithData{(unsigned char)startLabel, 0, 0}));

    size_t afterLoop = prog.size();
    prog[jeIdx].data = ArithData{(unsigned char)afterLoop, 0, 0};

    return 0;
}

BlockNode::BlockNode() { type = StatementType::BlockNode; }
int BlockNode::compile(std::vector<Instruction>& prog) const {
    for (const auto& s : statements) if (s) s->compile(prog);
    return 0;
}
void BlockNode::addStatement(std::unique_ptr<StatementNode> stmt) {
    statements.push_back(std::move(stmt));
}

SeqNode::SeqNode() { type = StatementType::SeqNode; }
int SeqNode::compile(std::vector<Instruction>& prog) const {
    for (const auto& s : statements) if (s) s->compile(prog);
    return 0;
}
void SeqNode::addStatement(std::unique_ptr<StatementNode> stmt) {
    statements.push_back(std::move(stmt));
}