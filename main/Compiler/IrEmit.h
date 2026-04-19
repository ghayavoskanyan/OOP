#pragma once
#include "VM.h"
#include <cstdint>
#include <vector>

inline void emitLoadImm(std::vector<Instruction>& prog, int reg, int32_t val) {
    if (val > 65535 || val < -32768) {
        int low  = val & 0xFFFF;
        int high = (val >> 16) & 0xFFFF;
        prog.push_back(Instruction(OpCode::LIL, LiData{(unsigned char)reg, (unsigned int)low}));
        prog.push_back(Instruction(OpCode::LIH, LiData{(unsigned char)reg, (unsigned int)high}));
    } else {
        prog.push_back(Instruction(OpCode::LI, LiData{(unsigned char)reg, (unsigned int)val}));
    }
}
