#include "VmMonitor.h"
#include "ExeImage.h"
#include <cstdlib>
#include <iostream>
#include <cstring>

std::vector<uint8_t> VmMonitor::buildImage(const std::vector<uint32_t>& code, const std::vector<int32_t>& data,
                                           uint32_t maxVReg) {
    std::vector<uint8_t> mem(RiscvCpu::kMemSize, 0);

    uint32_t codeAddr = kCodeBase;
    for (uint32_t w : code) {
        if (codeAddr + 4 > mem.size()) break;
        std::memcpy(mem.data() + codeAddr, &w, 4);
        codeAddr += 4;
    }

    uint32_t p = kDataBase;
    for (int32_t v : data) {
        if (p + 4 > mem.size()) break;
        std::memcpy(mem.data() + p, &v, 4);
        p += 4;
    }

    uint32_t vrBytes = (maxVReg + 128u) * 4u;
    if (kVrBase + vrBytes < mem.size()) {
        std::memset(mem.data() + kVrBase, 0, vrBytes);
    }

    if (kFlagBase + 8 < mem.size()) {
        std::memset(mem.data() + kFlagBase, 0, 8);
    }

    return mem;
}

bool VmMonitor::runImage(const std::vector<uint8_t>& mem, uint32_t entryPc, size_t maxSteps, std::string& errOut) {
    RiscvCpu cpu(mem);
    const char* dbg = std::getenv("OOP_VM_DEBUG");
    if (dbg && std::string(dbg) == "1") cpu.setDebugTrace(true);
    cpu.setPc(entryPc);
    cpu.runUntilHalt(maxSteps);
    std::cout << "[VM] cycles executed: " << cpu.getCycleCount() << "\n";
    if (!cpu.getLastError().empty()) {
        errOut = cpu.getLastError();
        return false;
    }
    if (!cpu.isHalted()) {
        errOut = "VM did not halt (step limit?)";
        return false;
    }
    return true;
}

bool VmMonitor::runExeFile(const std::string& path, std::string& errOut) {
    std::vector<uint32_t> code;
    std::vector<int32_t> data;
    uint32_t maxVReg = 0;
    uint32_t entry = kDefaultEntryAddr;
    if (!readExeFile(path, code, data, maxVReg, entry)) {
        errOut = "Failed to read executable: " + path;
        return false;
    }
    auto mem = buildImage(code, data, maxVReg);
    return runImage(mem, entry, 100000000, errOut);
}

bool VmMonitor::debugStep(RiscvCpu& cpu, std::string& errOut) {
    auto s = cpu.step();
    if (s == CpuStatus::Error) {
        errOut = cpu.getLastError();
        return false;
    }
    return true;
}
