#pragma once
#include "RiscvCpu.h"
#include <cstdint>
#include <set>
#include <string>
#include <vector>

class Debugger {
public:
    static constexpr uint32_t kCodeBase = 0x1000u;

    bool loadExe(const std::string& path, std::string& errOut);
    void runLoop();

private:
    std::vector<uint8_t> mem_;
    RiscvCpu cpu_{std::vector<uint8_t>{}};
    uint32_t entryPc_{kCodeBase};
    uint32_t codeBase_{kCodeBase};
    uint32_t codeEnd_{0};
    std::set<uint32_t> breakpoints_;
    bool loaded_{false};

    static constexpr size_t kMaxRunSteps = 100000000;

    uint32_t resolveAddr(uint32_t addr) const;
    uint32_t insnIndex(uint32_t pc) const;

    void showState() const;
    void showHelp() const;
    bool atBreakpoint() const;
    bool handleCommand(const std::string& line);
    bool stepInto(std::string& err);
    bool stepOver(std::string& err);
    bool stepOut(std::string& err);
    bool continueRun(std::string& err);
    bool runUntil(std::string& err, int targetDepth, bool stopOnBp);
};
