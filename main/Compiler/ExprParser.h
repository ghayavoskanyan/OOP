#pragma once
#include "ASTNode.h"
#include "Lexer.h"
#include "SymbolTable.h"
#include "TypeRegistry.h"
#include <memory>

namespace expr_parser {

std::unique_ptr<ASTNode> parseExpression(Lexer& lexer, SymbolTable& sym, TypeRegistry& types,
                                         bool stopAtCloseParen = false);

}
