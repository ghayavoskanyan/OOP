#pragma once
#include "ASTNode.h"
#include "Lexer.h"
#include "SymbolTable.h"
#include <memory>

namespace expr_parser {

std::unique_ptr<ASTNode> parseExpression(Lexer& lexer, SymbolTable& sym, bool stopAtCloseParen = false);

}
