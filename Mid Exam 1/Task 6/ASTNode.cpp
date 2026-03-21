/*ASTNode.cpp*/ /*էստեղ է AST-ն վերածվում հրահանգների, որոնք հետո VM-ը պիտի կարդա*/
#include "ASTNode.h"
#include <iostream>

// NumberNode 
NumberNode::NumberNode(double v) : value(v) { 
    type = NodeType::NumberNode; 
}

/*Ասում է VM-ին վերցրու այս թիվը և պահիր idx համարի տակ։ Վերադարձնում է այն ինդեքսի համարը, 
որտեղ պահվեց թիվը */
int NumberNode::compile(std::vector<Instruction>& prog) const {
    int idx = (int)prog.size();
    // MOV const -> rv
    prog.push_back(Instruction(OperandType::CONST, (int)value, OperandType::REG, idx));
    
    std::cout << "[COMPILE] Number " << value << " -> MOV const[" << (int)value 
              << "] -> rv[" << idx << "]" << std::endl;
    return idx;
}

// VariableNode 
VariableNode::VariableNode(const std::string& n, SymbolTable& sym)
    : name(n), symbolTable(sym) {
    type = NodeType::VariableNode;
}

/*Հարցնում է symbolTable-ին որ համարի տակ է պահված փոփոխականը։ Հետո VM-ին 
ասում է գնա այդ համարի տեղը ու բեր այդտեղի թիվը*/
int VariableNode::compile(std::vector<Instruction>& prog) const {
    int idx = (int)prog.size();
    int varIdx = symbolTable.getIndex(name);
    // MOV var -> rv
    prog.push_back(Instruction(OperandType::VAR, varIdx, OperandType::REG, idx));
    
    std::cout << "[COMPILE] Variable " << name << " (var[" << varIdx << "]) -> MOV var[" 
              << varIdx << "] -> rv[" << idx << "]" << std::endl;
    return idx;
}

// BinaryOpNode 
BinaryOpNode::BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r) 
    : op(o), left(std::move(l)), right(std::move(r)) { 
    type = NodeType::BinaryOpNode; 
}

/*Եթե ունեմ 5 + 3, նախ սարքում է հրահանգ 5-ի համար, հետո 3-ի համար ու վերջում 
ասում է՝ գումարիր այն, ինչ ստացար lIdx և rIdx տեղերում*/
int BinaryOpNode::compile(std::vector<Instruction>& prog) const {
    std::cout << "[COMPILE] BinaryOp " << op << " - compiling left operand...\n";
    int lIdx = left->compile(prog); //Կոմպիլյացիա արա ձախ կողմը
    
    std::cout << "[COMPILE] BinaryOp " << op << " - compiling right operand...\n";
    int rIdx = right->compile(prog); //Կոմպիլյացիա արա աջ կողմը
    
    //Որոշում է, թե ինչ գործողություն է + - * /

    int resIdx = (int)prog.size();
    OpCode code;
    std::string opName;
    switch(op) {
        case '+': code = OpCode::ADD; opName = "ADD"; break;
        case '-': code = OpCode::SUB; opName = "SUB"; break;
        case '*': code = OpCode::MUL; opName = "MUL"; break;
        case '/': code = OpCode::DIV; opName = "DIV"; break;
        default: code = OpCode::ADD; opName = "ADD";
    }
    /*Այս ֆայլը ծառը քանդում է և դարձնում գծային հրահանգներ*/
    
    prog.push_back(Instruction(code, lIdx, rIdx, resIdx));
    
    std::cout << "[COMPILE] BinaryOp " << op << " -> " << opName << " rv[" << lIdx << "], rv[" << rIdx << "] -> rv[" << resIdx << "]\n";
    return resIdx;
}

// UnaryOpNode 
UnaryOpNode::UnaryOpNode(char o, std::unique_ptr<ASTNode> node) : op(o), operand(std::move(node)) { 
    type = NodeType::UnaryOpNode; 
}

/*VM-ը չգիտի ինչ է -5-ը, դրա համար մենք ասում ենք վերցրու 0.0, հանիր նրանից 5-ը ու կստանաս -5*/
int UnaryOpNode::compile(std::vector<Instruction>& prog) const {
    std::cout << "[COMPILE] UnaryOp " << op << " - compiling operand...\n";
    int valIdx = operand->compile(prog);

    if (op == '-') {
        int zeroIdx = (int)prog.size();
        prog.push_back(Instruction(OperandType::CONST, 0, OperandType::REG, zeroIdx));
        std::cout << "[COMPILE] UnaryOp - -> MOV const[0] -> rv[" << zeroIdx << "]\n";

        int resIdx = (int)prog.size();
        prog.push_back(Instruction(OpCode::SUB, zeroIdx, valIdx, resIdx));
        
        std::cout << "[COMPILE] UnaryOp - -> SUB rv[" << zeroIdx << "], rv[" << valIdx << "] -> rv[" << resIdx << "]\n";
        return resIdx;
    }
    std::cout << "[COMPILE] UnaryOp + -> no change, using rv[" << valIdx << "]\n";
    return valIdx;
}

// AssignmentNode
AssignmentNode::AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym) : varName(name), expression(std::move(expr)), symbolTable(sym) {
    type = NodeType::AssignmentNode;
}

/*Օրինակ w = 10։ Նախ հաշվում է 10-ը, հետո STORE_VAR հրահանգով այդ 10-ը տեղափոխում է այնտեղ, որտեղ w-ն է ապրում*/
int AssignmentNode::compile(std::vector<Instruction>& prog) const {
    std::cout << "[COMPILE] Assignment " << varName << " = ... - compiling expression...\n";
    int exprIdx = expression->compile(prog); //Հաշվիր աջ կողմի արտահայտությունը
    int varIdx = symbolTable.getIndex(varName); //Գտիր փոփոխականի տեղը
    prog.push_back(Instruction(OperandType::REG, exprIdx, OperandType::VAR, varIdx));
    
    std::cout << "[COMPILE] Assignment " << varName << " -> MOV rv[" << exprIdx << "] -> var[" << varIdx << "] (" << varName << ")\n";
    return varIdx;
}