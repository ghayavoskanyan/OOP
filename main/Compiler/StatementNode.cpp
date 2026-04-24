#include "StatementNode.h"
#include "CompileRegs.h"
#include "IrEmit.h"
#include <iostream>
#include <stdexcept>

ExprStatement::ExprStatement(std::unique_ptr<ASTNode> e) {
    expr = std::move(e);
    type = StatementType::ExprStatement;
}

int ExprStatement::compile(std::vector<Instruction>& prog) const {
    if (expr) return expr->compile(prog);
    return 0;
}

PrintNode::PrintNode(std::unique_ptr<ASTNode> e, SymbolTable& sym) : symbolTable(sym) {
    expr = std::move(e);
    type = StatementType::PrintNode;
}

int PrintNode::compile(std::vector<Instruction>& prog) const {
    int reg = expr->compile(prog);
    prog.push_back(Instruction(OpCode::PRINT, ArithData{static_cast<uint32_t>(reg), 0, 0}));
    return reg;
}

DeclNode::DeclNode(const std::string& name, std::unique_ptr<ASTNode> init, SymbolTable& sym, bool isStatic)
    : varName(name), symbolTable(sym), isStatic_(isStatic) {
    initializer = std::move(init);
    type = StatementType::DeclNode;
    if (!symbolTable.hasSymbol(name))
        symbolTable.addSymbol(name);
}

int DeclNode::compile(std::vector<Instruction>& prog) const {
    int reg;
    if (initializer) {
        reg = initializer->compile(prog);
    } else {
        reg = compile_regs::newReg();
        emitLoadImm(prog, reg, 0);
    }
    int idx = (int)symbolTable.getIndex(varName);
    prog.push_back(Instruction(OpCode::STORE, ArithData{static_cast<uint32_t>(idx), 0, static_cast<uint32_t>(reg)}));
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
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, static_cast<uint32_t>(condReg), 0}));
    size_t jeIdx = prog.size();
    prog.push_back(Instruction(OpCode::JE, ArithData{0, 0, 0}));
    if (thenBody) thenBody->compile(prog);
    if (elseBody) {
        size_t jmpIdx = prog.size();
        prog.push_back(Instruction(OpCode::JMP, ArithData{0, 0, 0}));
        size_t elseStart = prog.size();
        prog[jeIdx].data = ArithData{static_cast<uint32_t>(elseStart), 0, 0};
        elseBody->compile(prog);
        size_t afterElse = prog.size();
        prog[jmpIdx].data = ArithData{static_cast<uint32_t>(afterElse), 0, 0};
    } else {
        size_t afterThen = prog.size();
        prog[jeIdx].data = ArithData{static_cast<uint32_t>(afterThen), 0, 0};
    }
    return 0;
}

WhileNode::WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> stmt) {
    condition = std::move(cond);
    body      = std::move(stmt);
    type = StatementType::WhileNode;
}

int WhileNode::compile(std::vector<Instruction>& prog) const {
    size_t startLabel = prog.size();
    int condReg = condition->compile(prog);
    prog.push_back(Instruction(OpCode::LI, LiData{0, 0}));
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, static_cast<uint32_t>(condReg), 0}));
    size_t jeIdx = prog.size();
    prog.push_back(Instruction(OpCode::JE, ArithData{0, 0, 0}));
    if (body) body->compile(prog);
    prog.push_back(Instruction(OpCode::JMP, ArithData{static_cast<uint32_t>(startLabel), 0, 0}));
    size_t afterLoop = prog.size();
    prog[jeIdx].data = ArithData{static_cast<uint32_t>(afterLoop), 0, 0};
    return 0;
}

ForNode::ForNode(std::unique_ptr<ASTNode> i, std::unique_ptr<ASTNode> c,
                 std::unique_ptr<ASTNode> u, std::unique_ptr<StatementNode> b) {
    init = std::move(i);
    condition = std::move(c);
    update = std::move(u);
    body = std::move(b);
    type = StatementType::ForNode;
}

int ForNode::compile(std::vector<Instruction>& prog) const {
    if (init) init->compile(prog);
    size_t startLabel = prog.size();
    int condReg = 0;
    if (condition) {
        condReg = condition->compile(prog);
    } else {
        condReg = compile_regs::newReg();
        emitLoadImm(prog, condReg, 1);
    }
    prog.push_back(Instruction(OpCode::LI, LiData{1, 0}));
    prog.push_back(Instruction(OpCode::CMP, ArithData{0, static_cast<uint32_t>(condReg), 1}));
    size_t jeIdx = prog.size();
    prog.push_back(Instruction(OpCode::JE, ArithData{0, 0, 0}));
    if (body) body->compile(prog);
    if (update) update->compile(prog);
    prog.push_back(Instruction(OpCode::JMP, ArithData{static_cast<uint32_t>(startLabel), 0, 0}));
    size_t afterLoop = prog.size();
    prog[jeIdx].data = ArithData{static_cast<uint32_t>(afterLoop), 0, 0};
    return 0;
}

BlockNode::BlockNode() { type = StatementType::BlockNode; }

int BlockNode::compile(std::vector<Instruction>& prog) const {
    for (const auto& s : statements)
        if (s) s->compile(prog);
    return 0;
}

void BlockNode::addStatement(std::unique_ptr<StatementNode> stmt) {
    statements.push_back(std::move(stmt));
}

SeqNode::SeqNode() { type = StatementType::SeqNode; }

int SeqNode::compile(std::vector<Instruction>& prog) const {
    for (const auto& s : statements)
        if (s) s->compile(prog);
    return 0;
}

void SeqNode::addStatement(std::unique_ptr<StatementNode> stmt) {
    statements.push_back(std::move(stmt));
}

FunctionDefNode::FunctionDefNode(std::string n, bool voidRet, std::vector<std::pair<std::string, std::string>> p,
                                 std::unique_ptr<StatementNode> b)
    : name(std::move(n)), returnVoid_(voidRet), params(std::move(p)), body(std::move(b)) {
    type = StatementType::FunctionDef;
}

int FunctionDefNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    return 0;
}

ReturnNode::ReturnNode(std::unique_ptr<ASTNode> e, bool voidRet) : expr(std::move(e)), voidReturn_(voidRet) {
    type = StatementType::ReturnStmt;
}

int ReturnNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("return is handled by the statement interpreter");
}

SwitchNode::SwitchNode(std::unique_ptr<ASTNode> disc, std::vector<SwitchArm> a)
    : discriminant(std::move(disc)), arms(std::move(a)) {
    type = StatementType::SwitchStmt;
}

int SwitchNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("switch is handled by the statement interpreter");
}

BreakNode::BreakNode() { type = StatementType::BreakStmt; }

int BreakNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("break is handled by the statement interpreter");
}

ContinueNode::ContinueNode() { type = StatementType::ContinueStmt; }

int ContinueNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("continue is handled by the statement interpreter");
}

GotoNode::GotoNode(std::string label) : label_(std::move(label)) { type = StatementType::GotoStmt; }

int GotoNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("goto is handled by the statement interpreter");
}

LabelNode::LabelNode(std::string label) : label_(std::move(label)) { type = StatementType::LabelStmt; }

int LabelNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    return 0;
}

DoWhileNode::DoWhileNode(std::unique_ptr<StatementNode> b, std::unique_ptr<ASTNode> c)
    : body(std::move(b)), condition(std::move(c)) {
    type = StatementType::DoWhileNode;
}

int DoWhileNode::compile(std::vector<Instruction>& prog) const {
    (void)prog;
    throw std::runtime_error("do-while is handled by the statement interpreter");
}
