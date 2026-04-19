#include "Traverser.h"
#include <stack>
#include <utility>

void Traverser::traverse(const ASTNode* node, std::function<void(const ASTNode*)> visitor) {
    if (!node) return;
    std::stack<std::pair<const ASTNode*, int>> stk;
    stk.push({node, 0});
    while (!stk.empty()) {
        auto& [current, state] = stk.top();
        if (state == 0) {
            state = 1;
            switch (current->type) {
                case NodeType::BinaryOpNode: {
                    auto* bin = static_cast<const BinaryOpNode*>(current);
                    if (bin->getRight()) stk.push({bin->getRight().get(), 0});
                    if (bin->getLeft())  stk.push({bin->getLeft().get(),  0});
                    break;
                }
                case NodeType::UnaryOpNode: {
                    auto* u = static_cast<const UnaryOpNode*>(current);
                    if (u->getOperand()) stk.push({u->getOperand().get(), 0});
                    break;
                }
                case NodeType::AssignmentNode: {
                    auto* a = static_cast<const AssignmentNode*>(current);
                    if (a->getExpression()) stk.push({a->getExpression().get(), 0});
                    break;
                }
                default: break;
            }
        } else {
            visitor(current);
            stk.pop();
        }
    }
}

void Traverser::traversePreOrder(const ASTNode* node, std::function<void(const ASTNode*)> visitor) {
    if (!node) return;
    std::stack<const ASTNode*> stk;
    stk.push(node);
    while (!stk.empty()) {
        const ASTNode* current = stk.top(); stk.pop();
        visitor(current);
        switch (current->type) {
            case NodeType::BinaryOpNode: {
                auto* bin = static_cast<const BinaryOpNode*>(current);
                if (bin->getRight()) stk.push(bin->getRight().get());
                if (bin->getLeft())  stk.push(bin->getLeft().get());
                break;
            }
            case NodeType::UnaryOpNode: {
                auto* u = static_cast<const UnaryOpNode*>(current);
                if (u->getOperand()) stk.push(u->getOperand().get());
                break;
            }
            case NodeType::AssignmentNode: {
                auto* a = static_cast<const AssignmentNode*>(current);
                if (a->getExpression()) stk.push(a->getExpression().get());
                break;
            }
            default: break;
        }
    }
}