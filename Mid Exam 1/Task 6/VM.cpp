/*շատ նման է CPU աշխատանքին*/
#include "VM.h"
#include <iostream>
#include <algorithm>

/*VM-ն ունի rv (Result Vector), որը նման է պրոցեսորի ռեգիստրներին։ Այս կոդը նախապես հաշվում է, 
թե ամենաշատը քանի տեղ է մեզ պետք գալու rv-ի մեջ, որպեսզի հանկարծ հիշողությունից դուրս չթռնենք*/
double VirtualMachine::execute(const std::vector<Instruction>& program, std::vector<double>& rv) {
    if (program.empty()) return 0.0;

    size_t maxIdx = 0;
    /*պատրաստում ենք հիշողություն*/
    for (const auto& instr : program) {
        if (instr.op == OpCode::MOV) {
            maxIdx = std::max(maxIdx, (size_t)std::max(instr.data.mov.srcIdx, instr.data.mov.dstIdx));
        } else {
            maxIdx = std::max(maxIdx, (size_t)std::max({instr.data.arith.left, instr.data.arith.right, instr.data.arith.resIdx}));
        }
    }
    
    if (rv.size() <= maxIdx) {
        rv.resize(maxIdx + 1, 0.0);
    }

    double lastResult = 0.0;

    /*VM-ը հերթով կարդում է program վեկտորի մեջ լցված հրահանգները։ Ամեն հրահանգ 
    ունի օպերատոր, left և right (թվերի տեղերը) և resIdx (որտեղ պահել պատասխանը)*/
    for (size_t i = 0; i < program.size(); i++) {
        const auto& instr = program[i];
        std::cout << "[VM] ";
        
        switch (instr.op) {
            case OpCode::MOV: {
                double srcVal = 0.0;
                switch (instr.data.mov.srcType) {
                    case OperandType::REG:
                        srcVal = rv[instr.data.mov.srcIdx];
                        std::cout << "MOV rv[" << instr.data.mov.srcIdx << "]";
                        break;
                    case OperandType::VAR:
                        srcVal = symbolTable.getValueByIndex(instr.data.mov.srcIdx);
                        std::cout << "MOV var[" << instr.data.mov.srcIdx << "]";
                        break;
                    case OperandType::CONST:
                        srcVal = (double)instr.data.mov.srcIdx;
                        std::cout << "MOV const[" << instr.data.mov.srcIdx << "]";
                        break;
                }
                
                std::cout << " -> ";
                
                switch (instr.data.mov.dstType) {
                    case OperandType::REG:
                        rv[instr.data.mov.dstIdx] = srcVal;
                        std::cout << "rv[" << instr.data.mov.dstIdx << "]";
                        lastResult = srcVal;
                        break;
                    case OperandType::VAR:
                        symbolTable.setValueByIndex(instr.data.mov.dstIdx, srcVal);
                        std::cout << "var[" << instr.data.mov.dstIdx << "]";
                        lastResult = srcVal;
                        break;
                    case OperandType::CONST:
                        throw std::runtime_error("Cannot write to constant");
                }
                std::cout << " = " << lastResult;
                break;
            }
            
            case OpCode::ADD:
                rv[instr.data.arith.resIdx] = rv[instr.data.arith.left] + rv[instr.data.arith.right];
                std::cout << "ADD rv[" << instr.data.arith.left << "] + rv[" 
                         << instr.data.arith.right << "]";
                lastResult = rv[instr.data.arith.resIdx];
                break;

            case OpCode::SUB:
                rv[instr.data.arith.resIdx] = rv[instr.data.arith.left] - rv[instr.data.arith.right];
                std::cout << "SUB rv[" << instr.data.arith.left << "] - rv[" 
                         << instr.data.arith.right << "]";
                lastResult = rv[instr.data.arith.resIdx];
                break;

            case OpCode::MUL:
                rv[instr.data.arith.resIdx] = rv[instr.data.arith.left] * rv[instr.data.arith.right];
                std::cout << "MUL rv[" << instr.data.arith.left << "] * rv[" 
                         << instr.data.arith.right << "]";
                lastResult = rv[instr.data.arith.resIdx];
                break;

            case OpCode::DIV:
                if (rv[instr.data.arith.right] == 0) {
                    throw std::runtime_error("Division by zero");
                }
                rv[instr.data.arith.resIdx] = rv[instr.data.arith.left] / rv[instr.data.arith.right];
                std::cout << "DIV rv[" << instr.data.arith.left << "] / rv[" 
                         << instr.data.arith.right << "]";
                lastResult = rv[instr.data.arith.resIdx];
                break;
        }

        std::cout << " = " << lastResult << "\n";
    }

    return lastResult;
}