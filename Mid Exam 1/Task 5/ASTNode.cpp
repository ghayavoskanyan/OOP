#include "ASTNode.h"

// --- NumberNode ---
NumberNode::NumberNode(double v) : value(v) { 
    type = NodeType::NumberNode; 
}

int NumberNode::compile(std::vector<Instruction>& prog) const {
    int idx = (int)prog.size();
    prog.push_back({OpCode::LOAD_CONST, value, -1, -1, idx});
    return idx;
}

// --- VariableNode ---
VariableNode::VariableNode(const std::string& n, SymbolTable& sym)
    : name(n), symbolTable(sym) {
    type = NodeType::VariableNode;
}

int VariableNode::compile(std::vector<Instruction>& prog) const {
    int idx = (int)prog.size();
    int varIdx = symbolTable.getIndex(name);
    prog.push_back({OpCode::LOAD_VAR, 0.0, varIdx, -1, idx});
    return idx;
}

// --- BinaryOpNode ---
BinaryOpNode::BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r) 
    : op(o), left(std::move(l)), right(std::move(r)) { 
    type = NodeType::BinaryOpNode; 
}

int BinaryOpNode::compile(std::vector<Instruction>& prog) const {
    int lIdx = left->compile(prog);
    int rIdx = right->compile(prog);
    
    int resIdx = (int)prog.size();
    OpCode code;
    switch(op) {
        case '+': code = OpCode::ADD; break;
        case '-': code = OpCode::SUB; break;
        case '*': code = OpCode::MUL; break;
        case '/': code = OpCode::DIV; break;
        default: code = OpCode::ADD;
    }
    
    prog.push_back({code, 0.0, lIdx, rIdx, resIdx});
    return resIdx;
}

// --- UnaryOpNode ---
UnaryOpNode::UnaryOpNode(char o, std::unique_ptr<ASTNode> node) 
    : op(o), operand(std::move(node)) { 
    type = NodeType::UnaryOpNode; 
}

int UnaryOpNode::compile(std::vector<Instruction>& prog) const {
    int valIdx = operand->compile(prog);

    if (op == '-') {
        // Create a constant 0 node
        int zeroIdx = (int)prog.size();
        prog.push_back({OpCode::LOAD_CONST, 0.0, -1, -1, zeroIdx});

        int resIdx = (int)prog.size();
        prog.push_back({OpCode::SUB, 0.0, zeroIdx, valIdx, resIdx});

        return resIdx;
    }
    // For unary plus, just return the value
    return valIdx;
}

// --- AssignmentNode ---
AssignmentNode::AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym)
    : varName(name), expression(std::move(expr)), symbolTable(sym) {
    type = NodeType::AssignmentNode;
}

int AssignmentNode::compile(std::vector<Instruction>& prog) const {
    int exprIdx = expression->compile(prog);
    int varIdx = symbolTable.getIndex(varName);
    prog.push_back({OpCode::STORE_VAR, 0.0, exprIdx, -1, varIdx});
    return varIdx;
}