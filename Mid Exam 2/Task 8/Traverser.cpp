/*Traverser.cpp*/ #include "Traverser.h"
#include <stack>
#include <utility>

void Traverser::traverse(const ASTNode* node, std::function<void(const ASTNode*)> visitor) {
    if (!node) return;
    
    std::stack<std::pair<const ASTNode*, int>> stack;
    stack.push({node, 0});
    
    while (!stack.empty()) {
        auto& [current, state] = stack.top();
        
        if (state == 0) {
            state = 1;
            
            switch (current->type) {
                case NodeType::BinaryOpNode: {
                    auto* bin = static_cast<const BinaryOpNode*>(current);
                    if (bin->getRight()) stack.push({bin->getRight().get(), 0});
                    if (bin->getLeft()) stack.push({bin->getLeft().get(), 0});
                    break;
                }
                case NodeType::UnaryOpNode: {
                    auto* unary = static_cast<const UnaryOpNode*>(current);
                    if (unary->getOperand()) stack.push({unary->getOperand().get(), 0});
                    break;
                }
                case NodeType::AssignmentNode: {
                    auto* assign = static_cast<const AssignmentNode*>(current);
                    if (assign->getExpression()) stack.push({assign->getExpression().get(), 0});
                    break;
                }
                default: break;
            }
        } else {
            visitor(current);
            stack.pop();
        }
    }
}

void Traverser::traversePreOrder(const ASTNode* node, std::function<void(const ASTNode*)> visitor) {
    if (!node) return;
    
    std::stack<const ASTNode*> stack;
    stack.push(node);
    
    while (!stack.empty()) {
        const ASTNode* current = stack.top();
        stack.pop();
        
        visitor(current);
        
        switch (current->type) {
            case NodeType::BinaryOpNode: {
                auto* bin = static_cast<const BinaryOpNode*>(current);
                if (bin->getRight()) stack.push(bin->getRight().get());
                if (bin->getLeft()) stack.push(bin->getLeft().get());
                break;
            }
            case NodeType::UnaryOpNode: {
                auto* unary = static_cast<const UnaryOpNode*>(current);
                if (unary->getOperand()) stack.push(unary->getOperand().get());
                break;
            }
            case NodeType::AssignmentNode: {
                auto* assign = static_cast<const AssignmentNode*>(current);
                if (assign->getExpression()) stack.push(assign->getExpression().get());
                break;
            }
            default: break;
        }
    }
}