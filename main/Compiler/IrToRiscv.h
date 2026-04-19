#pragma once
#include "VM.h"
#include <cstdint>
#include <vector>

namespace ir_to_riscv {

struct TranslationResult {
    std::vector<uint32_t> code;
    uint32_t maxVReg{0};
};

TranslationResult translate(const std::vector<Instruction>& program, uint32_t dataWordCount);

unsigned computeMaxVReg(const std::vector<Instruction>& program);

}
