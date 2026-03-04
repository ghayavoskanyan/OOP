#pragma once
#include <memory>
#include <string>
#include <vector>

enum class NodeType {
    NumberNode,
    VariableNode,
    BinaryOpNode,
    UnaryOpNode,
    AssignmentNode
};

class ASTNode {
public:
    NodeType type;
    virtual ~ASTNode() = default;
};

struct NumberNode : public ASTNode {
    double value;
    NumberNode(double v) : value(v) { type = NodeType::NumberNode; }
};

struct VariableNode : public ASTNode {
    std::string name;
    size_t symbolIndex;
    VariableNode(const std::string& n) : name(n), symbolIndex(0) { type = NodeType::VariableNode; }
};

struct BinaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    
    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {
        type = NodeType::BinaryOpNode;
    }
};

struct UnaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> operand;
    
    UnaryOpNode(char o, std::unique_ptr<ASTNode> node)
        : op(o), operand(std::move(node)) {
        type = NodeType::UnaryOpNode;
    }
};

struct AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> expression;
    
    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr)
        : varName(name), expression(std::move(expr)) {
        type = NodeType::AssignmentNode;
    }
};
