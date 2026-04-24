#pragma once
#include "ASTNode.h"
#include "VM.h"
#include "SymbolTable.h"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class StatementType {
    ExprStatement,
    IfNode,
    WhileNode,
    DoWhileNode,
    ForNode,
    BlockNode,
    SeqNode,
    PrintNode,
    DeclNode,
    FunctionDef,
    ReturnStmt,
    SwitchStmt,
    BreakStmt,
    ContinueStmt,
    GotoStmt,
    LabelStmt
};

class StatementNode {
public:
    StatementType type;
    virtual ~StatementNode() = default;
    virtual int compile(std::vector<Instruction>& prog) const = 0;
};

class ExprStatement : public StatementNode {
    std::unique_ptr<ASTNode> expr;

public:
    explicit ExprStatement(std::unique_ptr<ASTNode> e);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getExpr() const { return expr.get(); }
};

class PrintNode : public StatementNode {
    std::unique_ptr<ASTNode> expr;
    SymbolTable& symbolTable;

public:
    PrintNode(std::unique_ptr<ASTNode> e, SymbolTable& sym);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getExpr() const { return expr.get(); }
};

class DeclNode : public StatementNode {
    std::string varName;
    std::unique_ptr<ASTNode> initializer;
    SymbolTable& symbolTable;
    bool isStatic_;

public:
    DeclNode(const std::string& name, std::unique_ptr<ASTNode> init, SymbolTable& sym, bool isStatic = false);
    int compile(std::vector<Instruction>& prog) const override;
    const std::string& getVarName() const { return varName; }
    bool isStatic() const { return isStatic_; }
    bool hasInitializer() const { return initializer != nullptr; }
    const ASTNode* getInitializer() const { return initializer.get(); }
};

class FunctionDefNode : public StatementNode {
    std::string name;
    bool returnVoid_;
    std::vector<std::pair<std::string, std::string>> params;
    std::unique_ptr<StatementNode> body;

public:
    FunctionDefNode(std::string n, bool voidRet, std::vector<std::pair<std::string, std::string>> p,
                    std::unique_ptr<StatementNode> b);
    int compile(std::vector<Instruction>& prog) const override;
    const std::string& getName() const { return name; }
    bool isVoid() const { return returnVoid_; }
    const std::vector<std::pair<std::string, std::string>>& getParams() const { return params; }
    const StatementNode* getBody() const { return body.get(); }
    StatementNode* getBody() { return body.get(); }
};

class ReturnNode : public StatementNode {
    std::unique_ptr<ASTNode> expr;
    bool voidReturn_;

public:
    explicit ReturnNode(std::unique_ptr<ASTNode> e, bool voidRet = false);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getExpr() const { return expr.get(); }
    bool isVoidReturn() const { return voidReturn_; }
};


struct SwitchArm {
    bool isDefault{false};
    int32_t caseValue{0};
    std::vector<std::unique_ptr<StatementNode>> statements;
};

class SwitchNode : public StatementNode {
    std::unique_ptr<ASTNode> discriminant;
    std::vector<SwitchArm> arms;

public:
    SwitchNode(std::unique_ptr<ASTNode> disc, std::vector<SwitchArm> a);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getDiscriminant() const { return discriminant.get(); }
    const std::vector<SwitchArm>& getArms() const { return arms; }
};

class BreakNode : public StatementNode {
public:
    BreakNode();
    int compile(std::vector<Instruction>& prog) const override;
};

class ContinueNode : public StatementNode {
public:
    ContinueNode();
    int compile(std::vector<Instruction>& prog) const override;
};

class GotoNode : public StatementNode {
    std::string label_;

public:
    explicit GotoNode(std::string label);
    int compile(std::vector<Instruction>& prog) const override;
    const std::string& getLabel() const { return label_; }
};

class LabelNode : public StatementNode {
    std::string label_;

public:
    explicit LabelNode(std::string label);
    int compile(std::vector<Instruction>& prog) const override;
    const std::string& getLabel() const { return label_; }
};

class IfNode : public StatementNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<StatementNode> thenBody;
    std::unique_ptr<StatementNode> elseBody;

public:
    IfNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> thenStmt,
           std::unique_ptr<StatementNode> elseStmt);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getCondition() const { return condition.get(); }
    const StatementNode* getThen() const { return thenBody.get(); }
    const StatementNode* getElse() const { return elseBody.get(); }
};

class WhileNode : public StatementNode {
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<StatementNode> body;

public:
    WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<StatementNode> stmt);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getCondition() const { return condition.get(); }
    const StatementNode* getBody() const { return body.get(); }
};

class DoWhileNode : public StatementNode {
    std::unique_ptr<StatementNode> body;
    std::unique_ptr<ASTNode> condition;

public:
    DoWhileNode(std::unique_ptr<StatementNode> b, std::unique_ptr<ASTNode> c);
    int compile(std::vector<Instruction>& prog) const override;
    const StatementNode* getBody() const { return body.get(); }
    const ASTNode* getCondition() const { return condition.get(); }
};

class ForNode : public StatementNode {
    std::unique_ptr<ASTNode> init;
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> update;
    std::unique_ptr<StatementNode> body;

public:
    ForNode(std::unique_ptr<ASTNode> i, std::unique_ptr<ASTNode> c, std::unique_ptr<ASTNode> u,
            std::unique_ptr<StatementNode> b);
    int compile(std::vector<Instruction>& prog) const override;
    const ASTNode* getInit() const { return init.get(); }
    const ASTNode* getCondition() const { return condition.get(); }
    const ASTNode* getUpdate() const { return update.get(); }
    const StatementNode* getBody() const { return body.get(); }
};

class BlockNode : public StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;

public:
    BlockNode();
    int compile(std::vector<Instruction>& prog) const override;
    void addStatement(std::unique_ptr<StatementNode> stmt);
    const std::vector<std::unique_ptr<StatementNode>>& getStatements() const { return statements; }
};

class SeqNode : public StatementNode {
    std::vector<std::unique_ptr<StatementNode>> statements;

public:
    SeqNode();
    int compile(std::vector<Instruction>& prog) const override;
    void addStatement(std::unique_ptr<StatementNode> stmt);
    const std::vector<std::unique_ptr<StatementNode>>& getStatements() const { return statements; }
};
