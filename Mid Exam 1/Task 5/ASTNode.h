#pragma once
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "VM.h"

enum class NodeType { NumberNode, VariableNode, BinaryOpNode, UnaryOpNode, AssignmentNode };

class ASTNode {
public:
    NodeType type;
    virtual ~ASTNode() = default;
    virtual int compile(std::vector<Instruction>& prog) const = 0;
};

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double v);
    int compile(std::vector<Instruction>& prog) const override;
};

class VariableNode : public ASTNode {
public:
    std::string name;
    VariableNode(const std::string& n);
    int compile(std::vector<Instruction>& prog) const override;
};

class BinaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left, right;
public:
    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r);
    int compile(std::vector<Instruction>& prog) const override;
};

class UnaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> operand;
public:
    UnaryOpNode(char o, std::unique_ptr<ASTNode> node);
    int compile(std::vector<Instruction>& prog) const override;
};

class AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> expression;
public:
    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr);
    int compile(std::vector<Instruction>& prog) const override;
};