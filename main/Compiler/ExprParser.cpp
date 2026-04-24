#include "ExprParser.h"
#include <stdexcept>

namespace expr_parser {

static std::unique_ptr<ASTNode> parsePrimary(Lexer& lexer, SymbolTable& sym, TypeRegistry& types);
static std::unique_ptr<ASTNode> parsePostfix(Lexer& lexer, SymbolTable& sym, TypeRegistry& types, std::unique_ptr<ASTNode> e);
static std::unique_ptr<ASTNode> parseUnary(Lexer& lexer, SymbolTable& sym, TypeRegistry& types);
static std::unique_ptr<ASTNode> parseMultiplicative(Lexer& lexer, SymbolTable& sym, TypeRegistry& types);
static std::unique_ptr<ASTNode> parseAdditive(Lexer& lexer, SymbolTable& sym, TypeRegistry& types);
static std::unique_ptr<ASTNode> parseComparison(Lexer& lexer, SymbolTable& sym, TypeRegistry& types);
static std::unique_ptr<ASTNode> parseAssignment(Lexer& lexer, SymbolTable& sym, TypeRegistry& types);

static std::unique_ptr<ASTNode> parsePrimary(Lexer& lexer, SymbolTable& sym, TypeRegistry& types) {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Number) {
        if (t.value.find('.') != std::string::npos)
            throw std::runtime_error("Integer literals only: " + t.value);
        return std::make_unique<NumberNode>(static_cast<int32_t>(std::stol(t.value)));
    }
    if (t.type == TokenType::Name) {
        if (types.hasEnumConst(t.value)) {
            return std::make_unique<NumberNode>(types.getEnumConst(t.value));
        }
        Token la = lexer.getNextToken();
        if (la.type == TokenType::OpenParen) {
            std::vector<std::unique_ptr<ASTNode>> args;
            Token nx = lexer.getNextToken();
            if (nx.type == TokenType::CloseParen) {
                return std::make_unique<CallNode>(t.value, std::move(args), sym);
            }
            lexer.pushBack(nx);
            while (true) {
                args.push_back(parseExpression(lexer, sym, types, false));
                Token sep = lexer.getNextToken();
                if (sep.type == TokenType::CloseParen) break;
                if (sep.type != TokenType::Comma) throw std::runtime_error("Expected ',' or ')' in argument list");
            }
            return std::make_unique<CallNode>(t.value, std::move(args), sym);
        }
        lexer.pushBack(la);
        return std::make_unique<VariableNode>(t.value, sym);
    }
    if (t.type == TokenType::OpenParen) {
        auto e = parseExpression(lexer, sym, types, false);
        Token cp = lexer.getNextToken();
        if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')'");
        return e;
    }
    throw std::runtime_error("Unexpected token in expression: '" + t.value + "'");
}

static std::unique_ptr<ASTNode> parsePostfix(Lexer& lexer, SymbolTable& sym, TypeRegistry& types, std::unique_ptr<ASTNode> e) {
    while (true) {
        Token d = lexer.getNextToken();
        if (d.type != TokenType::Dot) {
            lexer.pushBack(d);
            break;
        }
        Token field = lexer.getNextToken();
        if (field.type != TokenType::Name)
            throw std::runtime_error("Expected field name after '.'");
        if (e->type != NodeType::VariableNode)
            throw std::runtime_error("Field access requires a simple variable (use struct variable)");
        std::string base = static_cast<VariableNode*>(e.get())->name;
        const std::string* instType = types.findInstanceType(base);
        if (instType) {
            const AggregateType* aggr = types.findAggregate(*instType);
            if (aggr && aggr->kind == AggregateKind::Class) {
                bool found = false;
                bool isPublic = false;
                for (const auto& f : aggr->fields) {
                    if (f.name == field.value) {
                        found = true;
                        isPublic = f.isPublic;
                        break;
                    }
                }
                if (found && !isPublic) throw std::runtime_error("Access to private class field: " + field.value);
            }
        }
        e = std::make_unique<VariableNode>(base + "." + field.value, sym);
    }
    return e;
}

static std::unique_ptr<ASTNode> parseUnary(Lexer& lexer, SymbolTable& sym, TypeRegistry& types) {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Keyword && t.value == "static_cast") {
        Token lt = lexer.getNextToken();
        if ((lt.type != TokenType::Operator && lt.type != TokenType::Comparison) || lt.value != "<")
            throw std::runtime_error("Expected '<' after static_cast");
        Token ty = lexer.getNextToken();
        if (!((ty.type == TokenType::Keyword && ty.value == "int") ||
              (ty.type == TokenType::Name && ty.value == "int")))
            throw std::runtime_error("static_cast requires int type in this compiler");
        Token gt = lexer.getNextToken();
        if ((gt.type != TokenType::Operator && gt.type != TokenType::Comparison) || gt.value != ">")
            throw std::runtime_error("Expected '>' after static_cast type");
        Token op = lexer.getNextToken();
        if (op.type != TokenType::OpenParen)
            throw std::runtime_error("Expected '(' after static_cast<...>");
        auto inner = parseExpression(lexer, sym, types, false);
        Token cl = lexer.getNextToken();
        if (cl.type != TokenType::CloseParen)
            throw std::runtime_error("Expected ')' after static_cast argument");
        return parsePostfix(lexer, sym, types, std::make_unique<CastNode>(std::move(inner)));
    }
    if (t.type == TokenType::Operator && t.value == "-") {
        auto inner = parseUnary(lexer, sym, types);
        return parsePostfix(lexer, sym, types, std::make_unique<UnaryOpNode>('-', std::move(inner)));
    }
    if (t.type == TokenType::Operator && t.value == "+") {
        return parseUnary(lexer, sym, types);
    }
    if (t.type == TokenType::OpenParen) {
        Token inner = lexer.getNextToken();
        if (inner.type == TokenType::Keyword && inner.value == "int") {
            Token cp = lexer.getNextToken();
            if (cp.type == TokenType::CloseParen)
                return parsePostfix(lexer, sym, types, std::make_unique<CastNode>(parseUnary(lexer, sym, types)));
            lexer.pushBack(cp);
        }
        lexer.pushBack(inner);
        lexer.pushBack(t);
        return parsePostfix(lexer, sym, types, parsePrimary(lexer, sym, types));
    }
    lexer.pushBack(t);
    return parsePostfix(lexer, sym, types, parsePrimary(lexer, sym, types));
}

static std::unique_ptr<ASTNode> parseMultiplicative(Lexer& lexer, SymbolTable& sym, TypeRegistry& types) {
    auto left = parseUnary(lexer, sym, types);
    while (true) {
        Token op = lexer.getNextToken();
        if (op.type != TokenType::Operator || (op.value != "*" && op.value != "/" && op.value != "%")) {
            lexer.pushBack(op);
            break;
        }
        char c = op.value[0];
        auto right = parseUnary(lexer, sym, types);
        left = std::make_unique<BinaryOpNode>(c, std::move(left), std::move(right));
    }
    return left;
}

static std::unique_ptr<ASTNode> parseAdditive(Lexer& lexer, SymbolTable& sym, TypeRegistry& types) {
    auto left = parseMultiplicative(lexer, sym, types);
    while (true) {
        Token op = lexer.getNextToken();
        if (op.type != TokenType::Operator || (op.value != "+" && op.value != "-")) {
            lexer.pushBack(op);
            break;
        }
        char c = op.value[0];
        auto right = parseMultiplicative(lexer, sym, types);
        left = std::make_unique<BinaryOpNode>(c, std::move(left), std::move(right));
    }
    return left;
}

static char cmpToOp(const std::string& s) {
    if (s == ">") return '>';
    if (s == "<") return '<';
    if (s == ">=") return 'G';
    if (s == "<=") return 'L';
    if (s == "==") return 'E';
    if (s == "!=") return 'N';
    return 0;
}

static std::unique_ptr<ASTNode> parseComparison(Lexer& lexer, SymbolTable& sym, TypeRegistry& types) {
    auto left = parseAdditive(lexer, sym, types);
    while (true) {
        Token op = lexer.getNextToken();
        if (op.type != TokenType::Comparison) {
            lexer.pushBack(op);
            break;
        }
        char c = cmpToOp(op.value);
        if (!c) throw std::runtime_error("Bad comparison op");
        auto right = parseAdditive(lexer, sym, types);
        left = std::make_unique<BinaryOpNode>(c, std::move(left), std::move(right));
    }
    return left;
}

static std::unique_ptr<ASTNode> parseAssignment(Lexer& lexer, SymbolTable& sym, TypeRegistry& types) {
    auto left = parseComparison(lexer, sym, types);
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Assignment) {
        if (left->type != NodeType::VariableNode)
            throw std::runtime_error("Left side of assignment must be a variable");
        std::string vn = static_cast<VariableNode*>(left.get())->name;
        auto rhs = parseAssignment(lexer, sym, types);
        return std::make_unique<AssignmentNode>(vn, std::move(rhs), sym);
    }
    lexer.pushBack(t);
    return left;
}

std::unique_ptr<ASTNode> parseExpression(Lexer& lexer, SymbolTable& sym, TypeRegistry& types, bool stopAtCloseParen) {
    auto e = parseAssignment(lexer, sym, types);
    if (stopAtCloseParen) {
        Token t = lexer.getNextToken();
        if (t.type != TokenType::CloseParen) {
            lexer.pushBack(t);
            throw std::runtime_error("Expected ')'");
        }
        lexer.pushBack(t);
    }
    return e;
}

} // namespace expr_parser
