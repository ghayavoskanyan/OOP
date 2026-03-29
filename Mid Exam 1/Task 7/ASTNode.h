/*ASTNode.h*/ #pragma once
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

class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double v);
    int compile(std::vector<Instruction>& prog) const override;
    double getValue() const { return value; }
};

class VariableNode : public ASTNode {
public:
    std::string name;
    SymbolTable& symbolTable;

    VariableNode(const std::string& n, SymbolTable& sym);
    int compile(std::vector<Instruction>& prog) const override;
    const std::string& getName() const { return name; }
};

class BinaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left, right;
public:
    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r);
    int compile(std::vector<Instruction>& prog) const override;
    char getOp() const { return op; }
    const std::unique_ptr<ASTNode>& getLeft() const { return left; }
    const std::unique_ptr<ASTNode>& getRight() const { return right; }
};

class UnaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> operand;
public:
    UnaryOpNode(char o, std::unique_ptr<ASTNode> node);
    int compile(std::vector<Instruction>& prog) const override;
    char getOp() const { return op; }
    const std::unique_ptr<ASTNode>& getOperand() const { return operand; }
};

class AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> expression;
    SymbolTable& symbolTable;
public:
    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym);
    int compile(std::vector<Instruction>& prog) const override;
    const std::string& getVarName() const { return varName; }
    const std::unique_ptr<ASTNode>& getExpression() const { return expression; }
};