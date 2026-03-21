/*էստեղ է AST-ն վերածվում հրահանգների, որոնք հետո VM-ը պիտի կարդա*/
#include "ASTNode.h"

// NumberNode 
NumberNode::NumberNode(double v) : value(v) { 
    type = NodeType::NumberNode; 
}

/*Ասում է VM-ին վերցրու այս թիվը և պահիր idx համարի տակ։ Վերադարձնում է այն ինդեքսի համարը, 
որտեղ պահվեց թիվը */
int NumberNode::compile(std::vector<Instruction>& prog) const {
    int idx = (int)prog.size();
    prog.push_back({OpCode::LOAD_CONST, value, -1, -1, idx});
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
    prog.push_back({OpCode::LOAD_VAR, 0.0, varIdx, -1, idx});
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
    int lIdx = left->compile(prog); //Կոմպիլյացիա արա ձախ կողմը
    int rIdx = right->compile(prog); //Կոմպիլյացիա արա աջ կողմը
    
    //Որոշում է, թե ինչ գործողություն է + - * /

    int resIdx = (int)prog.size();
    OpCode code;
    switch(op) {
        case '+': code = OpCode::ADD; break;
        case '-': code = OpCode::SUB; break;
        case '*': code = OpCode::MUL; break;
        case '/': code = OpCode::DIV; break;
        default: code = OpCode::ADD;
    }
    /*Այս ֆայլը ծառը քանդում է և դարձնում գծային հրահանգներ։ Թվերը դառնում են LOAD_CONST։
    Գումարումները դառնում են ADD։ Վերագրումը դառնում է STORE_VAR։*/
    
    prog.push_back({code, 0.0, lIdx, rIdx, resIdx});
    return resIdx;
}

// UnaryOpNode 
UnaryOpNode::UnaryOpNode(char o, std::unique_ptr<ASTNode> node) 
    : op(o), operand(std::move(node)) { 
    type = NodeType::UnaryOpNode; 
}

/*VM-ը չգիտի ինչ է -5-ը, դրա համար մենք ասում ենք վերցրու 0.0, հանիր նրանից 5-ը ու կստանաս -5*/
int UnaryOpNode::compile(std::vector<Instruction>& prog) const {
    int valIdx = operand->compile(prog);

    if (op == '-') {
        int zeroIdx = (int)prog.size();
        prog.push_back({OpCode::LOAD_CONST, 0.0, -1, -1, zeroIdx});

        int resIdx = (int)prog.size();
        prog.push_back({OpCode::SUB, 0.0, zeroIdx, valIdx, resIdx});

        return resIdx;
    }
    return valIdx;
}

// AssignmentNode
AssignmentNode::AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr, SymbolTable& sym)
    : varName(name), expression(std::move(expr)), symbolTable(sym) {
    type = NodeType::AssignmentNode;
}

/*Օրինակ w = 10։ Նախ հաշվում է 10-ը, հետո STORE_VAR հրահանգով այդ 10-ը տեղափոխում է այնտեղ, որտեղ w-ն է ապրում*/
int AssignmentNode::compile(std::vector<Instruction>& prog) const {
    int exprIdx = expression->compile(prog); //Հաշվիր աջ կողմի արտահայտությունը
    int varIdx = symbolTable.getIndex(varName); //Գտիր փոփոխականի տեղը
    prog.push_back({OpCode::STORE_VAR, 0.0, exprIdx, -1, varIdx}); //Պահիր արդյունքը փոփոխականի մեջ
    return varIdx;
}