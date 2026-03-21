#pragma once
#include <vector>
#include <stdexcept>
#include "SymbolTable.h"

enum class OpCode {
    MOV, //Load, Store քցեցինք մի տեղ 
    ADD,
    SUB,
    MUL,
    DIV
};

enum class OperandType {
    REG, //օպերանդը rv վեկտորի ինդեքս է
    VAR, //օպերանդը values վեկտորի ինդեքս է՝ փոփոխական
    CONST //օպերանդը ուղղակի թիվ է
};

struct Instruction {
    OpCode op;

    /*Union-ը թույլ է տալիս նույն հիշողության տարածքում պահել տարբեր տեսակի տվյալներ՝ կախված նրանից, թե ինչ հրահանգ է։
    Եթե op == MOV, ապա օգտագործում ենք mov struct-ը, որը պարունակում է աղբյուրի և նպատակակետի տիպերն ու ինդեքսները,
    եթե op == ADD/SUB/MUL/DIV, ապա օգտագործում ենք arith struct-ը, որը պարունակում է երկու օպերանդների և արդյունքի ինդեքսները։
    Սա հիշողության օպտիմալացում է -> չենք պահում ավելորդ դաշտեր։*/
    union {
        struct {
            OperandType srcType;
            int srcIdx;
            OperandType dstType;
            int dstIdx;
        } mov;
        struct {
            int left;
            int right;
            int resIdx;
        } arith;
        double constVal; 
    } data;
    
    Instruction(OperandType srcT, int src, OperandType dstT, int dst) : op(OpCode::MOV) {
        data.mov.srcType = srcT;
        data.mov.srcIdx = src;
        data.mov.dstType = dstT;
        data.mov.dstIdx = dst;
    }
    
    Instruction(OpCode o, int l, int r, int res) : op(o) {
        data.arith.left = l;
        data.arith.right = r;
        data.arith.resIdx = res;
    }
};

class VirtualMachine {
    SymbolTable& symbolTable;

public:
    VirtualMachine(SymbolTable& sym) : symbolTable(sym) {}

    double execute(const std::vector<Instruction>& program, std::vector<double>& rv);
};