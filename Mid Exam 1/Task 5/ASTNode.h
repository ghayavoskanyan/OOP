#pragma once
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "VM.h"
#include "SymbolTable.h"

enum class NodeType { NumberNode, VariableNode, BinaryOpNode, UnaryOpNode, AssignmentNode };

class ASTNode {
public:
    NodeType type;
    virtual ~ASTNode() = default;
    virtual int compile(std::vector<Instruction>& prog) const = 0;
};

// --- Number Node ---
class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double v);
    int compile(std::vector<Instruction>& prog) const override;
};

// --- Variable Node ---
class VariableNode : public ASTNode {
public:
    std::string name;
    SymbolTable& symbolTable;

    VariableNode(const std::string& n, SymbolTable& sym);
    int compile(std::vector<Instruction>& prog) const override;
};

// --- Binary Operation Node ---
class BinaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left, right;
public:
    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r);
    int compile(std::vector<Instruction>& prog) const override;
};

// --- Unary Operation Node ---
class UnaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> operand;
public:
    UnaryOpNode(char o, std::unique_ptr<ASTNode> node);
    int compile(std::vector<Instruction>& prog) const override;
};

// --- Assignment Node ---
class AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> expression;
    SymbolTable& symbolTable;
public:
    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym);
    int compile(std::vector<Instruction>& prog) const override;
};