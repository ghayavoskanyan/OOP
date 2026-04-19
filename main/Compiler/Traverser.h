#pragma once
#include "ASTNode.h"
#include <functional>

class Traverser {
public:
    void traverse(const ASTNode* node, std::function<void(const ASTNode*)> visitor);
    void traversePreOrder(const ASTNode* node, std::function<void(const ASTNode*)> visitor);
};