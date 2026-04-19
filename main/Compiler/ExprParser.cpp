#include "ExprParser.h"
#include <stdexcept>

namespace expr_parser {

static std::unique_ptr<ASTNode> parsePrimary(Lexer& lexer, SymbolTable& sym);
static std::unique_ptr<ASTNode> parseUnary(Lexer& lexer, SymbolTable& sym);
static std::unique_ptr<ASTNode> parseMultiplicative(Lexer& lexer, SymbolTable& sym);
static std::unique_ptr<ASTNode> parseAdditive(Lexer& lexer, SymbolTable& sym);
static std::unique_ptr<ASTNode> parseComparison(Lexer& lexer, SymbolTable& sym);
static std::unique_ptr<ASTNode> parseAssignment(Lexer& lexer, SymbolTable& sym);

static std::unique_ptr<ASTNode> parsePrimary(Lexer& lexer, SymbolTable& sym) {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Number) {
        if (t.value.find('.') != std::string::npos)
            throw std::runtime_error("Integer literals only: " + t.value);
        return std::make_unique<NumberNode>(static_cast<int32_t>(std::stol(t.value)));
    }
    if (t.type == TokenType::Name) {
        Token la = lexer.getNextToken();
        if (la.type == TokenType::OpenParen) {
            std::vector<std::unique_ptr<ASTNode>> args;
            Token nx = lexer.getNextToken();
            if (nx.type == TokenType::CloseParen) {
                return std::make_unique<CallNode>(t.value, std::move(args), sym);
            }
            lexer.pushBack(nx);
            while (true) {
                args.push_back(parseExpression(lexer, sym, false));
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
        auto e = parseExpression(lexer, sym, false);
        Token cp = lexer.getNextToken();
        if (cp.type != TokenType::CloseParen) throw std::runtime_error("Expected ')'");
        return e;
    }
    throw std::runtime_error("Unexpected token in expression: '" + t.value + "'");
}

static std::unique_ptr<ASTNode> parseUnary(Lexer& lexer, SymbolTable& sym) {
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Operator && t.value == "-") {
        auto inner = parseUnary(lexer, sym);
        return std::make_unique<UnaryOpNode>('-', std::move(inner));
    }
    if (t.type == TokenType::Operator && t.value == "+") {
        return parseUnary(lexer, sym);
    }
    lexer.pushBack(t);
    return parsePrimary(lexer, sym);
}

static std::unique_ptr<ASTNode> parseMultiplicative(Lexer& lexer, SymbolTable& sym) {
    auto left = parseUnary(lexer, sym);
    while (true) {
        Token op = lexer.getNextToken();
        if (op.type != TokenType::Operator || (op.value != "*" && op.value != "/" && op.value != "%")) {
            lexer.pushBack(op);
            break;
        }
        char c = op.value[0];
        auto right = parseUnary(lexer, sym);
        left = std::make_unique<BinaryOpNode>(c, std::move(left), std::move(right));
    }
    return left;
}

static std::unique_ptr<ASTNode> parseAdditive(Lexer& lexer, SymbolTable& sym) {
    auto left = parseMultiplicative(lexer, sym);
    while (true) {
        Token op = lexer.getNextToken();
        if (op.type != TokenType::Operator || (op.value != "+" && op.value != "-")) {
            lexer.pushBack(op);
            break;
        }
        char c = op.value[0];
        auto right = parseMultiplicative(lexer, sym);
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

static std::unique_ptr<ASTNode> parseComparison(Lexer& lexer, SymbolTable& sym) {
    auto left = parseAdditive(lexer, sym);
    while (true) {
        Token op = lexer.getNextToken();
        if (op.type != TokenType::Comparison) {
            lexer.pushBack(op);
            break;
        }
        char c = cmpToOp(op.value);
        if (!c) throw std::runtime_error("Bad comparison op");
        auto right = parseAdditive(lexer, sym);
        left = std::make_unique<BinaryOpNode>(c, std::move(left), std::move(right));
    }
    return left;
}

static std::unique_ptr<ASTNode> parseAssignment(Lexer& lexer, SymbolTable& sym) {
    auto left = parseComparison(lexer, sym);
    Token t = lexer.getNextToken();
    if (t.type == TokenType::Assignment) {
        if (left->type != NodeType::VariableNode)
            throw std::runtime_error("Left side of assignment must be a variable");
        std::string vn = static_cast<VariableNode*>(left.get())->name;
        auto rhs = parseAssignment(lexer, sym);
        return std::make_unique<AssignmentNode>(vn, std::move(rhs), sym);
    }
    lexer.pushBack(t);
    return left;
}

std::unique_ptr<ASTNode> parseExpression(Lexer& lexer, SymbolTable& sym, bool stopAtCloseParen) {
    auto e = parseAssignment(lexer, sym);
    if (stopAtCloseParen) {
        Token t = lexer.getNextToken();
        if (t.type != TokenType::CloseParen) {
            lexer.pushBack(t);
            throw std::runtime_error("Expected ')'");
        }
    }
    return e;
}

} // namespace expr_parser
