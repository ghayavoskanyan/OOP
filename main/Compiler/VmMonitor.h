#pragma once
#include "RiscvCpu.h"
#include <cstdint>
#include <string>
#include <vector>

class VmMonitor {
public:
    static constexpr uint32_t kCodeBase = 0x1000u;
    static constexpr uint32_t kDataBase = 0x10000u;
    static constexpr uint32_t kVrBase = 0x20000u;
    static constexpr uint32_t kFlagBase = 0x21000u;

    static std::vector<uint8_t> buildImage(const std::vector<uint32_t>& code, const std::vector<int32_t>& data,
                                           uint32_t maxVReg);

    static bool runImage(const std::vector<uint8_t>& mem, uint32_t entryPc, size_t maxSteps, std::string& errOut);

    static bool runExeFile(const std::string& path, std::string& errOut);

    static bool debugStep(RiscvCpu& cpu, std::string& errOut);
};
