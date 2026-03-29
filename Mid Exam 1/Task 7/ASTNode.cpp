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
    int intVal = (int)value;
    // LI (Load Immediate) - const -> r
    if (intVal > 65535) {
        int low = intVal & 0xFFFF;
        int high = (intVal >> 16) & 0xFFFF;
        prog.push_back(Instruction(OpCode::LIL, LiData{(unsigned char)idx, (unsigned int)low}));
        prog.push_back(Instruction(OpCode::LIH, LiData{(unsigned char)idx, (unsigned int)high}));
    } else {
        prog.push_back(Instruction(OpCode::LI, LiData{(unsigned char)idx, (unsigned int)intVal}));
    }
    
    std::cout << "[COMPILE] Number " << value << " -> LI r" << idx << ", " << intVal << std::endl;
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
    // LOAD var -> r
    prog.push_back(Instruction(OpCode::LOAD, ArithData{(unsigned char)idx, 0, (unsigned char)varIdx}));
    
    std::cout << "[COMPILE] Variable " << name << " (var[" << varIdx << "]) -> LOAD r" << idx << ", [v" << varIdx << "]" << std::endl;
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
    int lIdx = left->compile(prog);
    
    std::cout << "[COMPILE] BinaryOp " << op << " - compiling right operand...\n";
    int rIdx = right->compile(prog);
    
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
    
    prog.push_back(Instruction(code, ArithData{(unsigned char)resIdx, (unsigned char)lIdx, (unsigned char)rIdx}));
    
    std::cout << "[COMPILE] BinaryOp " << op << " -> " << opName << " r" << resIdx << ", r" << lIdx << ", r" << rIdx << "\n";
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
        prog.push_back(Instruction(OpCode::LI, LiData{(unsigned char)zeroIdx, 0}));
        std::cout << "[COMPILE] UnaryOp - -> LI r" << zeroIdx << ", 0\n";

        int resIdx = (int)prog.size();
        prog.push_back(Instruction(OpCode::SUB, ArithData{(unsigned char)resIdx, (unsigned char)zeroIdx, (unsigned char)valIdx}));
        
        std::cout << "[COMPILE] UnaryOp - -> SUB r" << resIdx << ", r" << zeroIdx << ", r" << valIdx << "\n";
        return resIdx;
    }
    std::cout << "[COMPILE] UnaryOp + -> no change, using r" << valIdx << "\n";
    return valIdx;
}

// AssignmentNode
AssignmentNode::AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym) : varName(name), expression(std::move(expr)), symbolTable(sym) {
    type = NodeType::AssignmentNode;
}

/*Օրինակ w = 10։ Նախ հաշվում է 10-ը, հետո STORE_VAR հրահանգով այդ 10-ը տեղափոխում է այնտեղ, որտեղ w-ն է ապրում*/
int AssignmentNode::compile(std::vector<Instruction>& prog) const {
    std::cout << "[COMPILE] Assignment " << varName << " = ... - compiling expression...\n";
    int exprIdx = expression->compile(prog);
    int varIdx = symbolTable.getIndex(varName);
    // STORE: r -> var (LOAD-ով ենք անում, բայց destination-ը var-ն է)
    prog.push_back(Instruction(OpCode::LOAD, ArithData{(unsigned char)varIdx, 0, (unsigned char)exprIdx}));
    
    std::cout << "[COMPILE] Assignment " << varName << " -> LOAD var[" << varIdx << "], r" << exprIdx << "\n";
    return varIdx;
}