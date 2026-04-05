/*StatementNode.h*/ #pragma once
#include "ASTNode.h"
#include "VM.h"
#include "SymbolTable.h"
#include <memory>
#include <vector>

enum class StatementType { IfNode, WhileNode, BlockNode, SeqNode };

class StatementNode {
public:
    StatementType type;
    virtual ~StatementNode() = default;
    virtual int compile(std::vector<Instruction>& prog) const = 0;
};

class IfNode : public StatementNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<StatementNode> thenBody;
    std::unique_ptr<StatementNode> elseBody;
public:
    IfNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> thenStmt, std::unique_ptr<StatementNode> elseStmt);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getCondition() const { return condition.get(); }
};

class WhileNode : public StatementNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<StatementNode> body;
public:
    WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> stmt);
    int compile(std::vector<Instruction>& prog) const override;
};

class BlockNode : public StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
public:
    BlockNode();
    int compile(std::vector<Instruction>& prog) const override;
    void addStatement(std::unique_ptr<StatementNode> stmt);
};

class SeqNode : public StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;
public:
    SeqNode();
    int compile(std::vector<Instruction>& prog) const override;
    void addStatement(std::unique_ptr<StatementNode> stmt);
};