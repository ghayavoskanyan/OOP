#pragma once
#include <cstdint>
#include <string>
#include <vector>

enum class CpuStatus { Running, Halted, Error };

class RiscvCpu {
public:
    static constexpr uint32_t kMemSize = 1u << 24;

    explicit RiscvCpu(std::vector<uint8_t> memory);

    void setPc(uint32_t pc);
    uint32_t getPc() const { return pc_; }
    int32_t getReg(unsigned r) const;
    void setReg(unsigned r, int32_t v);

    CpuStatus step();
    void runUntilHalt(size_t maxSteps = 100000000);
    void setDebugTrace(bool on) { debugTrace_ = on; }
    uint64_t getCycleCount() const { return cycleCount_; }

    const std::string& getLastError() const { return lastError_; }
    bool isHalted() const { return halted_; }

private:
    std::vector<uint8_t> mem_;
    int32_t x_[32]{};
    uint32_t pc_{0};
    bool halted_{false};
    std::string lastError_;
    bool debugTrace_{false};
    uint64_t cycleCount_{0};

    uint32_t loadWord(uint32_t addr) const;
    void storeWord(uint32_t addr, uint32_t val);
    void illegal(uint32_t insn);
};
