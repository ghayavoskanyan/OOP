#include "ASTNode.h"

NumberNode::NumberNode(double v) : value(v) { type = NodeType::NumberNode; }
int NumberNode::compile(std::vector<Instruction>& prog) const {
    int idx = (int)prog.size();
    prog.push_back({OpCode::LOAD_CONST, value, -1, -1, idx});
    return idx;
}

VariableNode::VariableNode(const std::string& n) : name(n) { type = NodeType::VariableNode; }
int VariableNode::compile(std::vector<Instruction>& prog) const {
    return 0; 
}

BinaryOpNode::BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r) 
    : op(o), left(std::move(l)), right(std::move(r)) { type = NodeType::BinaryOpNode; }

int BinaryOpNode::compile(std::vector<Instruction>& prog) const {
    int lIdx = left->compile(prog);
    int rIdx = right->compile(prog);
    
    int resIdx = (int)prog.size();
    OpCode code = (op == '+') ? OpCode::ADD : (op == '-') ? OpCode::SUB : 
                  (op == '*') ? OpCode::MUL : OpCode::DIV;
    
    prog.push_back({code, 0.0, lIdx, rIdx, resIdx});
    return resIdx;
}

UnaryOpNode::UnaryOpNode(char o, std::unique_ptr<ASTNode> node) 
    : op(o), operand(std::move(node)) { type = NodeType::UnaryOpNode; }

int UnaryOpNode::compile(std::vector<Instruction>& prog) const {
    int valIdx = operand->compile(prog);

    if (op == '-' || op == 'u') {
        int zeroIdx = (int)prog.size();
        prog.push_back({OpCode::LOAD_CONST, 0.0, -1, -1, zeroIdx});

        int resIdx = (int)prog.size();
        prog.push_back({OpCode::SUB, 0.0, zeroIdx, valIdx, resIdx});

        return resIdx;
    }

    return valIdx;
}


AssignmentNode::AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr) 
    : varName(name), expression(std::move(expr)) { type = NodeType::AssignmentNode; }

int AssignmentNode::compile(std::vector<Instruction>& prog) const {
    return expression->compile(prog);
}