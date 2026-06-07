#include "RiscvCpu.h"
#include <iomanip>
#include <iostream>
#include <sstream>

RiscvCpu::RiscvCpu(std::vector<uint8_t> memory) : mem_(std::move(memory)) {
    if (mem_.size() < kMemSize) mem_.resize(kMemSize, 0);
    x_[0] = 0;
}

void RiscvCpu::setPc(uint32_t pc) {
    pc_ = pc;
    halted_ = false;
    lastError_.clear();
}

int32_t RiscvCpu::getReg(unsigned r) const {
    if (r >= 32) return 0;
    return r == 0 ? 0 : x_[r];
}

void RiscvCpu::setReg(unsigned r, int32_t v) {
    if (r == 0 || r >= 32) return;
    x_[r] = v;
}

uint32_t RiscvCpu::peekInsn() const {
    return loadWord(pc_);
}

int32_t RiscvCpu::readMemWord(uint32_t addr) const {
    return (int32_t)loadWord(addr);
}

bool RiscvCpu::isCallInsn(uint32_t insn) {
    uint32_t opcode = insn & 0x7F;
    uint32_t rd = (insn >> 7) & 0x1F;
    uint32_t funct3 = (insn >> 12) & 7;
    if (rd == 0) return false;
    if (opcode == 0x6F) return true;
    if (opcode == 0x67 && funct3 == 0) return true;
    return false;
}

bool RiscvCpu::isReturnInsn(uint32_t insn) {
    uint32_t opcode = insn & 0x7F;
    uint32_t rd = (insn >> 7) & 0x1F;
    uint32_t funct3 = (insn >> 12) & 7;
    return opcode == 0x67 && rd == 0 && funct3 == 0;
}

std::string RiscvCpu::disassemble(uint32_t insn, uint32_t pc) {
    auto regName = [](uint32_t r) {
        static const char* names[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                                      "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                                      "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                                      "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
        return (r < 32) ? names[r] : "?";
    };

    uint32_t opcode = insn & 0x7F;
    uint32_t rd = (insn >> 7) & 0x1F;
    uint32_t rs1 = (insn >> 15) & 0x1F;
    uint32_t rs2 = (insn >> 20) & 0x1F;
    uint32_t funct3 = (insn >> 12) & 7;
    uint32_t funct7 = (insn >> 25) & 0x7F;

    std::ostringstream os;
    os << std::hex << std::setfill('0') << "0x" << std::setw(8) << insn << std::dec << "  ";

    if (insn == 0x00000073u) { os << "ecall"; return os.str(); }
    if (insn == 0x00100073u) { os << "ebreak"; return os.str(); }

    switch (opcode) {
        case 0x37: {
            uint32_t imm = insn & 0xFFFFF000u;
            os << "lui " << regName(rd) << ", 0x" << std::hex << (imm >> 12) << std::dec;
            break;
        }
        case 0x17: {
            uint32_t imm = insn & 0xFFFFF000u;
            os << "auipc " << regName(rd) << ", 0x" << std::hex << (imm >> 12) << std::dec;
            break;
        }
        case 0x6F: {
            uint32_t i = insn;
            uint32_t raw = (((i >> 31) & 1) << 20) | (((i >> 21) & 0x3FF) << 1) | (((i >> 20) & 1) << 11) | (((i >> 12) & 0xFF) << 12);
            int32_t imm = (int32_t)((raw << 11) >> 11);
            os << "jal " << regName(rd) << ", " << (pc + imm);
            break;
        }
        case 0x67:
            os << "jalr " << regName(rd) << ", " << (int32_t)(insn >> 20) << "(" << regName(rs1) << ")";
            break;
        case 0x13:
            if (funct3 == 0) os << "addi " << regName(rd) << ", " << regName(rs1) << ", " << (int32_t)(insn >> 20);
            else if (funct3 == 1) os << "slli " << regName(rd) << ", " << regName(rs1) << ", " << ((insn >> 20) & 0x1F);
            else if (funct3 == 3) os << "sltiu " << regName(rd) << ", " << regName(rs1) << ", " << (int32_t)(insn >> 20);
            else if (funct3 == 7) os << "andi " << regName(rd) << ", " << regName(rs1) << ", " << (int32_t)(insn >> 20);
            else os << "addi?";
            break;
        case 0x03:
            if (funct3 == 2) os << "lw " << regName(rd) << ", " << (int32_t)(insn >> 20) << "(" << regName(rs1) << ")";
            else os << "load?";
            break;
        case 0x23:
            if (funct3 == 2) os << "sw " << regName(rs2) << ", " << (int32_t)(((insn >> 25) << 5) | ((insn >> 7) & 0x1F)) << "(" << regName(rs1) << ")";
            else os << "store?";
            break;
        case 0x33:
            if (funct7 == 0x01) {
                if (funct3 == 0) os << "mul " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else if (funct3 == 4) os << "div " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else if (funct3 == 6) os << "rem " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else os << "mul?";
            } else if (funct7 == 0) {
                if (funct3 == 0) os << "add " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else if (funct3 == 2) os << "slt " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else if (funct3 == 6) os << "or " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else if (funct3 == 7) os << "and " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
                else os << "alu?";
            } else if (funct7 == 0x20 && funct3 == 0) os << "sub " << regName(rd) << ", " << regName(rs1) << ", " << regName(rs2);
            else os << "alu?";
            break;
        case 0x63: {
            const char* br = (funct3 == 0) ? "beq" : (funct3 == 1) ? "bne" : (funct3 == 4) ? "blt" : (funct3 == 5) ? "bge" : "br?";
            uint32_t bi = insn;
            uint32_t braw = ((bi >> 31) << 12) | (((bi >> 7) & 1) << 11) | (((bi >> 25) & 0x3F) << 5) | (((bi >> 8) & 0xF) << 1);
            int32_t imm = (int32_t)((braw << 19) >> 19);
            os << br << " " << regName(rs1) << ", " << regName(rs2) << ", " << (pc + imm);
            break;
        }
        default:
            os << "unknown";
            break;
    }
    return os.str();
}

void RiscvCpu::updateCallDepth(uint32_t insn) {
    if (isCallInsn(insn)) {
        callDepth_++;
    } else if (isReturnInsn(insn) && callDepth_ > 0) {
        callDepth_--;
    }
}

uint32_t RiscvCpu::loadWord(uint32_t addr) const {
    if (addr + 4 > mem_.size()) return 0;
    uint32_t v = 0;
    v |= (uint32_t)mem_[addr];
    v |= (uint32_t)mem_[addr + 1] << 8;
    v |= (uint32_t)mem_[addr + 2] << 16;
    v |= (uint32_t)mem_[addr + 3] << 24;
    return v;
}

void RiscvCpu::storeWord(uint32_t addr, uint32_t val) {
    if (addr + 4 > mem_.size()) return;
    mem_[addr]     = (uint8_t)(val & 0xFF);
    mem_[addr + 1] = (uint8_t)((val >> 8) & 0xFF);
    mem_[addr + 2] = (uint8_t)((val >> 16) & 0xFF);
    mem_[addr + 3] = (uint8_t)((val >> 24) & 0xFF);
}

void RiscvCpu::illegal(uint32_t insn) {
    lastError_ = "Illegal or unsupported instruction at PC=0x" + std::to_string(pc_) + " word=0x" + std::to_string(insn);
    halted_ = true;
}

CpuStatus RiscvCpu::step() {
    if (halted_) return CpuStatus::Halted;
    if (pc_ + 4 > mem_.size()) {
        lastError_ = "PC out of memory";
        halted_ = true;
        return CpuStatus::Error;
    }

    uint32_t insn = loadWord(pc_);
    if (debugTrace_) {
        std::cout << "[PC=0x" << std::hex << pc_ << "] insn=0x" << insn << std::dec << "\n";
    }
    uint32_t opcode = insn & 0x7F;
    uint32_t rd = (insn >> 7) & 0x1F;
    uint32_t rs1 = (insn >> 15) & 0x1F;
    uint32_t rs2 = (insn >> 20) & 0x1F;
    uint32_t funct3 = (insn >> 12) & 7;
    uint32_t funct7 = (insn >> 25) & 0x7F;

    auto immI = [&]() -> int32_t {
        return (int32_t)insn >> 20;
    };
    auto immS = [&]() -> int32_t {
        int32_t imm = (int32_t)(((insn >> 25) << 5) | ((insn >> 7) & 0x1F));
        imm = (imm << 20) >> 20;
        return imm;
    };
    auto immB = [&]() -> int32_t {
        uint32_t i = insn;
        uint32_t imm = ((i >> 31) << 12) | (((i >> 7) & 1) << 11) | (((i >> 25) & 0x3F) << 5) | (((i >> 8) & 0xF) << 1);
        return (int32_t)((imm << 19) >> 19);
    };
    auto immU = [&]() -> uint32_t { return insn & 0xFFFFF000u; };
    auto immJ = [&]() -> int32_t {
        uint32_t i = insn;
        uint32_t imm = (((i >> 31) & 1) << 20) | (((i >> 21) & 0x3FF) << 1) | (((i >> 20) & 1) << 11) | (((i >> 12) & 0xFF) << 12);
        return (int32_t)((imm << 11) >> 11);
    };

    uint32_t nextPc = pc_ + 4;

    switch (opcode) {
        case 0x33: { 
            int32_t v1 = getReg(rs1);
            int32_t v2 = getReg(rs2);
            uint32_t u1 = (uint32_t)v1;
            uint32_t u2 = (uint32_t)v2;
            int32_t out = 0;
            if (funct7 == 0x01) { // M extension
                if (funct3 == 0) out = (int32_t)(u1 * u2);
                else if (funct3 == 4) {
                    if (v2 == 0) {
                        lastError_ = "Division by zero";
                        halted_ = true;
                        return CpuStatus::Error;
                    }
                    out = v1 / v2;
                } else if (funct3 == 6) {
                    if (v2 == 0) out = v1;
                    else out = v1 % v2;
                } else {
                    illegal(insn);
                    return CpuStatus::Error;
                }
            } else if (funct7 == 0) {
                if (funct3 == 0) out = v1 + v2;
                else if (funct3 == 2) out = (v1 < v2) ? 1 : 0;
                else if (funct3 == 6) out = v1 | v2;
                else if (funct3 == 7) out = v1 & v2;
                else {
                    illegal(insn);
                    return CpuStatus::Error;
                }
            } else if (funct7 == 0x20 && funct3 == 0) {
                out = v1 - v2;
            } else {
                illegal(insn);
                return CpuStatus::Error;
            }
            setReg(rd, out);
            break;
        }
        case 0x13: { 
            int32_t v1 = getReg(rs1);
            int32_t imm = immI();
            if (funct3 == 0) setReg(rd, v1 + imm);
            else if (funct3 == 1 && funct7 == 0) {
                uint32_t sh = (insn >> 20) & 0x1F;
                setReg(rd, (int32_t)((uint32_t)v1 << sh));
            } else if (funct3 == 3) setReg(rd, ((uint32_t)v1 < (uint32_t)imm) ? 1 : 0);
            else if (funct3 == 7) setReg(rd, v1 & imm);
            else {
                illegal(insn);
                return CpuStatus::Error;
            }
            break;
        }
        case 0x03: { 
            int32_t base = getReg(rs1);
            int32_t addr = base + immI();
            if (funct3 == 2) {
                if ((addr & 3) != 0) {
                    lastError_ = "Misaligned load";
                    halted_ = true;
                    return CpuStatus::Error;
                }
                setReg(rd, (int32_t)loadWord((uint32_t)addr));
            } else {
                illegal(insn);
                return CpuStatus::Error;
            }
            break;
        }
        case 0x23: { 
            int32_t base = getReg(rs1);
            int32_t addr = base + immS();
            int32_t val = getReg(rs2);
            if (funct3 == 2) {
                if ((addr & 3) != 0) {
                    lastError_ = "Misaligned store";
                    halted_ = true;
                    return CpuStatus::Error;
                }
                storeWord((uint32_t)addr, (uint32_t)val);
            } else {
                illegal(insn);
                return CpuStatus::Error;
            }
            break;
        }
        case 0x63: { 
            int32_t v1 = getReg(rs1);
            int32_t v2 = getReg(rs2);
            int32_t off = immB();
            bool take = false;
            if (funct3 == 0) take = (v1 == v2);
            else if (funct3 == 1) take = (v1 != v2);
            else if (funct3 == 4) take = (v1 < v2);
            else if (funct3 == 5) take = (v1 >= v2);
            else {
                illegal(insn);
                return CpuStatus::Error;
            }
            if (take) nextPc = pc_ + off;
            break;
        }
        case 0x37: { 
            setReg(rd, (int32_t)immU());
            break;
        }
        case 0x17: { 
            setReg(rd, (int32_t)(pc_ + immU()));
            break;
        }
        case 0x6F: { 
            int32_t off = immJ();
            setReg(rd, (int32_t)nextPc);
            nextPc = pc_ + off;
            break;
        }
        case 0x67: { 
            if (funct3 != 0) {
                illegal(insn);
                return CpuStatus::Error;
            }
            int32_t t = getReg(rs1) + immI();
            setReg(rd, (int32_t)nextPc);
            nextPc = (uint32_t)t & ~1u;
            break;
        }
        case 0x73: { 
            if (insn == 0x00100073u) { 
                halted_ = true;
                pc_ = nextPc;
                return CpuStatus::Halted;
            }
            if (insn == 0x00000073u) { 
                int32_t a7 = getReg(17);
                int32_t a0 = getReg(10);
                if (a7 == 1) {
                    std::cout << a0 << std::endl;
                } else if (a7 == 0) {
                    halted_ = true;
                    pc_ = nextPc;
                    return CpuStatus::Halted;
                } else {
                    lastError_ = "Unknown ECALL a7=" + std::to_string(a7);
                    halted_ = true;
                    return CpuStatus::Error;
                }
                break;
            }
            illegal(insn);
            return CpuStatus::Error;
        }
        default:
            illegal(insn);
            return CpuStatus::Error;
    }

    updateCallDepth(insn);
    x_[0] = 0;
    pc_ = nextPc;
    cycleCount_++;
    return CpuStatus::Running;
}

void RiscvCpu::runUntilHalt(size_t maxSteps) {
    cycleCount_ = 0;
    for (size_t i = 0; i < maxSteps && !halted_; ++i) {
        CpuStatus s = step();
        if (s != CpuStatus::Running) break;
    }
}
