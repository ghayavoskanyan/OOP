#pragma once
#include "ASTNode.h"
#include "StatementNode.h"
#include "SymbolTable.h"
#include "TypeRegistry.h"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

bool programNeedsInterpreter(const StatementNode* root);

class StmtInterpreter {
public:
    explicit StmtInterpreter(SymbolTable& globals);
    StmtInterpreter(SymbolTable& globals, TypeRegistry& types);

    int32_t run(const StatementNode* root);

private:
    SymbolTable& globals_;
    TypeRegistry* types_;
    std::unordered_map<std::string, const FunctionDefNode*> functions_;

    int32_t evalExpr(const ASTNode* node, std::unordered_map<std::string, int32_t>* locals);
    void execStmt(const StatementNode* stmt, std::unordered_map<std::string, int32_t>* locals, int32_t& returnValue,
                  bool& returned, bool& breakFlag, bool& continueFlag, std::string& gotoLabel);

    void collectFunctions(const StatementNode* stmt);
    void registerFunction(const FunctionDefNode* fn);
};
