#include "IrToRiscv.h"
#include "RiscvISA.h"
#include <algorithm>
#include <stdexcept>

namespace {

constexpr uint8_t T0 = 5, T1 = 6, T2 = 7, T3 = 28, T4 = 29, S0 = 8, S1 = 9, S2 = 18, A0 = 10, A7 = 17;

constexpr uint32_t kDataBase = 0x10000u;
constexpr uint32_t kVrBase = 0x20000u;
constexpr uint32_t kFlagBase = 0x21000u;

void emitSmallImm(std::vector<uint32_t>& c, uint8_t rd, int32_t val) {
    if (val >= -2048 && val <= 2047) {
        c.push_back(riscv::addi(rd, 0, val));
    } else {
        uint32_t hi = (uint32_t)((val + 0x800) >> 12);
        int32_t lo = val - (int32_t)(hi << 12);
        c.push_back(riscv::lui(rd, hi & 0xFFFFF));
        if (lo != 0) c.push_back(riscv::addi(rd, rd, lo));
    }
}

void emitLoadAddr(std::vector<uint32_t>& c, uint8_t rd, uint32_t addr) {
    uint32_t hi = (addr + 0x800u) >> 12;
    int32_t lo = (int32_t)(addr - (hi << 12));
    c.push_back(riscv::lui(rd, hi & 0xFFFFF));
    if (lo != 0) c.push_back(riscv::addi(rd, rd, lo));
}

void emitLoadImm32(std::vector<uint32_t>& c, uint8_t rd, int32_t v) { emitSmallImm(c, rd, v); }

void emitVrPtr(std::vector<uint32_t>& c, uint8_t out, unsigned vreg) {
    emitSmallImm(c, T0, (int32_t)vreg);
    c.push_back(riscv::slli(T0, T0, 2));
    c.push_back(riscv::add_(out, S1, T0));
}

void emitDataPtr(std::vector<uint32_t>& c, uint8_t out, unsigned varIdx) {
    int32_t off = (int32_t)(varIdx * 4u);
    if (off >= -2048 && off <= 2047) {
        c.push_back(riscv::addi(out, S0, off));
    } else {
        emitSmallImm(c, T0, off);
        c.push_back(riscv::add_(out, S0, T0));
    }
}

enum class BrKind { BEQ, BNE, BLT, BGE };

struct JalPatch {
    size_t at{};
    size_t targetIr{};
};
struct BrPatch {
    size_t at{};
    size_t targetIr{};
    BrKind k{};
    uint8_t rs1{};
    uint8_t rs2{};
};

void patchJals(std::vector<uint32_t>& code, const std::vector<size_t>& irStart, const std::vector<JalPatch>& patches) {
    for (const auto& p : patches) {
        int64_t delta = (int64_t)irStart[p.targetIr] - (int64_t)p.at;
        int32_t off = (int32_t)(delta * 4);
        code[p.at] = riscv::jal(0, off);
    }
}

void patchBrs(std::vector<uint32_t>& code, const std::vector<size_t>& irStart, const std::vector<BrPatch>& patches) {
    for (const auto& p : patches) {
        int64_t delta = (int64_t)irStart[p.targetIr] - (int64_t)p.at;
        int32_t off = (int32_t)(delta * 4);
        uint32_t w = 0;
        switch (p.k) {
            case BrKind::BEQ: w = riscv::beq(p.rs1, p.rs2, off); break;
            case BrKind::BNE: w = riscv::bne(p.rs1, p.rs2, off); break;
            case BrKind::BLT: w = riscv::blt(p.rs1, p.rs2, off); break;
            case BrKind::BGE: w = riscv::bge(p.rs1, p.rs2, off); break;
        }
        code[p.at] = w;
    }
}

} // namespace

unsigned ir_to_riscv::computeMaxVReg(const std::vector<Instruction>& program) {
    unsigned m = 0;
    for (const auto& ins : program) {
        switch (ins.opcode) {
            case OpCode::LI:
            case OpCode::LIL:
            case OpCode::LIH: {
                auto& d = std::get<LiData>(ins.data);
                m = (std::max)(m, (unsigned)d.dest);
                break;
            }
            default: {
                auto& d = std::get<ArithData>(ins.data);
                m = (std::max)(m, (unsigned)d.dest);
                m = (std::max)(m, (unsigned)d.left);
                m = (std::max)(m, (unsigned)d.right);
                break;
            }
        }
    }
    return m;
}

ir_to_riscv::TranslationResult ir_to_riscv::translate(const std::vector<Instruction>& program, uint32_t) {
    TranslationResult res;
    auto& code = res.code;
    std::vector<JalPatch> jalPatches;
    std::vector<BrPatch> brPatches;

    emitLoadAddr(code, S0, kDataBase);
    emitLoadAddr(code, S1, kVrBase);
    emitLoadAddr(code, S2, kFlagBase);

    std::vector<size_t> irStart;
    irStart.reserve(program.size() + 1);

    for (size_t ir = 0; ir < program.size(); ++ir) {
        irStart.push_back(code.size());
        const Instruction& ins = program[ir];

        switch (ins.opcode) {
            case OpCode::LI: {
                auto& d = std::get<LiData>(ins.data);
                emitLoadImm32(code, T0, (int32_t)d.value);
                emitVrPtr(code, T1, d.dest);
                code.push_back(riscv::sw(T0, T1, 0));
                break;
            }
            case OpCode::LIL: {
                auto& d = std::get<LiData>(ins.data);
                int32_t v = (int32_t)(d.value & 0xFFFF);
                emitLoadImm32(code, T0, v);
                emitVrPtr(code, T1, d.dest);
                code.push_back(riscv::sw(T0, T1, 0));
                break;
            }
            case OpCode::LIH: {
                auto& d = std::get<LiData>(ins.data);
                int32_t addv = (int32_t)((d.value & 0xFFFF) * 65536);
                emitVrPtr(code, T1, d.dest);
                code.push_back(riscv::lw(T0, T1, 0));
                emitSmallImm(code, T2, addv);
                code.push_back(riscv::add_(T0, T0, T2));
                code.push_back(riscv::sw(T0, T1, 0));
                break;
            }
            case OpCode::LOAD: {
                auto& d = std::get<ArithData>(ins.data);
                emitDataPtr(code, T2, d.right);
                code.push_back(riscv::lw(T0, T2, 0));
                emitVrPtr(code, T1, d.dest);
                code.push_back(riscv::sw(T0, T1, 0));
                break;
            }
            case OpCode::STORE: {
                auto& d = std::get<ArithData>(ins.data);
                emitVrPtr(code, T1, d.right);
                code.push_back(riscv::lw(T0, T1, 0));
                emitDataPtr(code, T2, d.dest);
                code.push_back(riscv::sw(T0, T2, 0));
                break;
            }
            case OpCode::ADD:
            case OpCode::SUB:
            case OpCode::MUL:
            case OpCode::DIV: {
                auto& d = std::get<ArithData>(ins.data);
                emitVrPtr(code, T1, d.left);
                code.push_back(riscv::lw(T0, T1, 0));
                emitVrPtr(code, T2, d.right);
                code.push_back(riscv::lw(T1, T2, 0));
                if (ins.opcode == OpCode::ADD) code.push_back(riscv::add_(T2, T0, T1));
                else if (ins.opcode == OpCode::SUB) code.push_back(riscv::sub_(T2, T0, T1));
                else if (ins.opcode == OpCode::MUL) code.push_back(riscv::mul_(T2, T0, T1));
                else code.push_back(riscv::div_(T2, T0, T1));
                emitVrPtr(code, T0, d.dest);
                code.push_back(riscv::sw(T2, T0, 0));
                break;
            }
            case OpCode::CMP: {
                auto& d = std::get<ArithData>(ins.data);
                emitVrPtr(code, T1, d.left);
                code.push_back(riscv::lw(T0, T1, 0));
                emitVrPtr(code, T2, d.right);
                code.push_back(riscv::lw(T1, T2, 0));
                code.push_back(riscv::sub_(T2, T0, T1));
                code.push_back(riscv::sltiu(T3, T2, 1));
                code.push_back(riscv::slt_(T4, T0, T1));
                code.push_back(riscv::sw(T3, S2, 0));
                code.push_back(riscv::sw(T4, S2, 4));
                break;
            }
            case OpCode::JMP: {
                auto& d = std::get<ArithData>(ins.data);
                size_t at = code.size();
                code.push_back(riscv::jal(0, 0));
                jalPatches.push_back({at, (size_t)d.dest});
                break;
            }
            case OpCode::JE: {
                auto& d = std::get<ArithData>(ins.data);
                code.push_back(riscv::lw(T0, S2, 0));
                emitSmallImm(code, T1, 1);
                size_t at = code.size();
                code.push_back(riscv::beq(T0, T1, 0));
                brPatches.push_back({at, (size_t)d.dest, BrKind::BEQ, T0, T1});
                break;
            }
            case OpCode::JNE: {
                auto& d = std::get<ArithData>(ins.data);
                code.push_back(riscv::lw(T0, S2, 0));
                size_t at = code.size();
                code.push_back(riscv::beq(T0, 0, 0));
                brPatches.push_back({at, (size_t)d.dest, BrKind::BEQ, T0, 0});
                break;
            }
            case OpCode::JG: {
                auto& d = std::get<ArithData>(ins.data);
                code.push_back(riscv::lw(T0, S2, 0));
                code.push_back(riscv::lw(T1, S2, 4));
                code.push_back(riscv::or_(T2, T0, T1));
                size_t at = code.size();
                code.push_back(riscv::beq(T2, 0, 0));
                brPatches.push_back({at, (size_t)d.dest, BrKind::BEQ, T2, 0});
                break;
            }
            case OpCode::JL: {
                auto& d = std::get<ArithData>(ins.data);
                code.push_back(riscv::lw(T1, S2, 4));
                size_t at = code.size();
                code.push_back(riscv::bne(T1, 0, 0));
                brPatches.push_back({at, (size_t)d.dest, BrKind::BNE, T1, 0});
                break;
            }
            case OpCode::JGE: {
                auto& d = std::get<ArithData>(ins.data);
                code.push_back(riscv::lw(T1, S2, 4));
                size_t at = code.size();
                code.push_back(riscv::beq(T1, 0, 0));
                brPatches.push_back({at, (size_t)d.dest, BrKind::BEQ, T1, 0});
                break;
            }
            case OpCode::JLE: {
                auto& d = std::get<ArithData>(ins.data);
                code.push_back(riscv::lw(T0, S2, 0));
                code.push_back(riscv::lw(T1, S2, 4));
                code.push_back(riscv::or_(T2, T0, T1));
                size_t at = code.size();
                code.push_back(riscv::bne(T2, 0, 0));
                brPatches.push_back({at, (size_t)d.dest, BrKind::BNE, T2, 0});
                break;
            }
            case OpCode::PRINT: {
                auto& d = std::get<ArithData>(ins.data);
                emitVrPtr(code, T1, d.dest);
                code.push_back(riscv::lw(A0, T1, 0));
                emitSmallImm(code, A7, 1);
                code.push_back(riscv::ecall());
                break;
            }
            default:
                throw std::runtime_error("IrToRiscv: unsupported opcode");
        }
    }

    irStart.push_back(code.size());

    patchJals(code, irStart, jalPatches);
    patchBrs(code, irStart, brPatches);

    emitSmallImm(code, A7, 0);
    code.push_back(riscv::ecall());

    res.maxVReg = computeMaxVReg(program);
    (void)kVrBase;
    (void)kFlagBase;
    return res;
}
