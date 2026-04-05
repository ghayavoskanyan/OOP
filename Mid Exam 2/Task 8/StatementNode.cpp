/*StatementNode.cpp*/ #include "StatementNode.h"
#include <iostream>

IfNode::IfNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> thenStmt, std::unique_ptr<StatementNode> elseStmt)
    : condition(std::move(cond)), thenBody(std::move(thenStmt)), elseBody(std::move(elseStmt)) {
    type = StatementType::IfNode;
}

int IfNode::compile(std::vector<Instruction>& prog) const {
    std::cout << "[COMPILE] IfNode\n";
    int condReg = condition->compile(prog);
    int elseLabel = (int)prog.size() + 2;
    int endLabel = (int)prog.size() + 3;
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, (unsigned char)condReg, 0}));
    prog.push_back(Instruction(OpCode::JE, ArithData{(unsigned char)elseLabel, 0, 0}));
    if (thenBody) thenBody->compile(prog);
    prog.push_back(Instruction(OpCode::JMP, ArithData{(unsigned char)endLabel, 0, 0}));
    if (elseBody) elseBody->compile(prog);
    return 0;
}

WhileNode::WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> stmt)
    : condition(std::move(cond)), body(std::move(stmt)) {
    type = StatementType::WhileNode;
}

int WhileNode::compile(std::vector<Instruction>& prog) const {
    int startLabel = (int)prog.size();
    int condReg = condition->compile(prog);
    int endLabel = (int)prog.size() + 2;
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, (unsigned char)condReg, 0}));
    prog.push_back(Instruction(OpCode::JE, ArithData{(unsigned char)endLabel, 0, 0}));
    if (body) body->compile(prog);
    prog.push_back(Instruction(OpCode::JMP, ArithData{(unsigned char)startLabel, 0, 0}));
    return 0;
}

BlockNode::BlockNode() { type = StatementType::BlockNode; }
int BlockNode::compile(std::vector<Instruction>& prog) const {
    for (const auto& stmt : statements) if (stmt) stmt->compile(prog);
    return 0;
}
void BlockNode::addStatement(std::unique_ptr<StatementNode> stmt) { statements.push_back(std::move(stmt)); }

SeqNode::SeqNode() { type = StatementType::SeqNode; }
int SeqNode::compile(std::vector<Instruction>& prog) const {
    for (const auto& stmt : statements) if (stmt) stmt->compile(prog);
    return 0;
}
void SeqNode::addStatement(std::unique_ptr<StatementNode> stmt) { statements.push_back(std::move(stmt)); }