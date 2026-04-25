#pragma once
#include <cstdint>

namespace riscv {

inline uint32_t lui(uint8_t rd, uint32_t imm20) {
    return 0x37u | ((uint32_t)rd << 7) | ((imm20 & 0xFFFFFu) << 12);
}

inline uint32_t auipc(uint8_t rd, uint32_t imm20) {
    return 0x17u | ((uint32_t)rd << 7) | ((imm20 & 0xFFFFFu) << 12);
}

inline uint32_t encodeI(uint32_t imm12, uint8_t funct3, uint8_t rs1, uint8_t rd, uint8_t opcode) {
    imm12 &= 0xFFFu;
    return (uint32_t)opcode | ((uint32_t)rd << 7) | ((uint32_t)funct3 << 12) | ((uint32_t)rs1 << 15) | (imm12 << 20);
}

inline uint32_t addi(uint8_t rd, uint8_t rs1, int32_t imm12) { return encodeI((uint32_t)(imm12 & 0xFFF), 0, rs1, rd, 0x13); }

inline uint32_t sltiu(uint8_t rd, uint8_t rs1, int32_t imm12) { return encodeI((uint32_t)(imm12 & 0xFFF), 3, rs1, rd, 0x13); }

inline uint32_t andi(uint8_t rd, uint8_t rs1, int32_t imm12) { return encodeI((uint32_t)(imm12 & 0xFFF), 7, rs1, rd, 0x13); }

inline uint32_t slli(uint8_t rd, uint8_t rs1, uint32_t shamt) {
    uint32_t imm = (shamt & 0x1Fu);
    return encodeI(imm, 1, rs1, rd, 0x13);
}

inline uint32_t lw(uint8_t rd, uint8_t rs1, int32_t imm12) { return encodeI((uint32_t)(imm12 & 0xFFF), 2, rs1, rd, 0x03); }

inline uint32_t sw(uint8_t rs2, uint8_t rs1, int32_t imm12) {
    uint32_t imm = (uint32_t)(imm12 & 0xFFF);
    uint32_t imm11_5 = (imm >> 5) & 0x7F;
    uint32_t imm4_0 = imm & 0x1F;
    return 0x23u | (imm4_0 << 7) | (2u << 12) | ((uint32_t)rs1 << 15) | ((uint32_t)rs2 << 20) | (imm11_5 << 25);
}

inline uint32_t encodeR(uint8_t funct7, uint8_t funct3, uint8_t rs1, uint8_t rs2, uint8_t rd, uint8_t opcode) {
    return (uint32_t)opcode | ((uint32_t)rd << 7) | ((uint32_t)funct3 << 12) | ((uint32_t)rs1 << 15) | ((uint32_t)rs2 << 20) | ((uint32_t)funct7 << 25);
}

inline uint32_t add_(uint8_t rd, uint8_t rs1, uint8_t rs2) { return encodeR(0, 0, rs1, rs2, rd, 0x33); }

inline uint32_t sub_(uint8_t rd, uint8_t rs1, uint8_t rs2) { return encodeR(0x20, 0, rs1, rs2, rd, 0x33); }

inline uint32_t slt_(uint8_t rd, uint8_t rs1, uint8_t rs2) { return encodeR(0, 2, rs1, rs2, rd, 0x33); }

inline uint32_t or_(uint8_t rd, uint8_t rs1, uint8_t rs2) { return encodeR(0, 6, rs1, rs2, rd, 0x33); }

inline uint32_t mul_(uint8_t rd, uint8_t rs1, uint8_t rs2) { return encodeR(0x01, 0, rs1, rs2, rd, 0x33); }

inline uint32_t div_(uint8_t rd, uint8_t rs1, uint8_t rs2) { return encodeR(0x01, 4, rs1, rs2, rd, 0x33); }

inline uint32_t jal(uint8_t rd, int32_t offset) {
    uint32_t imm = (uint32_t)offset;
    uint32_t insn = 0x6Fu | ((uint32_t)rd << 7);
    insn |= ((imm >> 12) & 0xFFu) << 12;
    insn |= ((imm >> 11) & 1u) << 20;
    insn |= ((imm >> 1) & 0x3FFu) << 21;
    insn |= ((imm >> 20) & 1u) << 31;
    return insn;
}

inline uint32_t jalr(uint8_t rd, uint8_t rs1, int32_t imm12) { return encodeI((uint32_t)(imm12 & 0xFFF), 0, rs1, rd, 0x67); }

inline uint32_t branch(uint8_t funct3, uint8_t rs1, uint8_t rs2, int32_t offset) {
    uint32_t imm = (uint32_t)offset;
    uint32_t insn = 0x63u | ((uint32_t)funct3 << 12) | ((uint32_t)rs1 << 15) | ((uint32_t)rs2 << 20);
    insn |= (((imm >> 11) & 1u) << 7);
    insn |= (((imm >> 1) & 0xFu) << 8);
    insn |= (((imm >> 5) & 0x3Fu) << 25);
    insn |= (((imm >> 12) & 1u) << 31);
    return insn;
}

inline uint32_t beq(uint8_t rs1, uint8_t rs2, int32_t byteOff) { return branch(0, rs1, rs2, byteOff); }

inline uint32_t bne(uint8_t rs1, uint8_t rs2, int32_t byteOff) { return branch(1, rs1, rs2, byteOff); }

inline uint32_t blt(uint8_t rs1, uint8_t rs2, int32_t byteOff) { return branch(4, rs1, rs2, byteOff); }

inline uint32_t bge(uint8_t rs1, uint8_t rs2, int32_t byteOff) { return branch(5, rs1, rs2, byteOff); }

inline uint32_t ecall() { return 0x00000073u; }

inline uint32_t ebreak() { return 0x00100073u; }

} 
