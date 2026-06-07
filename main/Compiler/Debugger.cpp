#include "Debugger.h"
#include "ExeImage.h"
#include "VmMonitor.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

namespace {

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::string toLower(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tok;
    std::istringstream iss(line);
    std::string t;
    while (iss >> t) tok.push_back(t);
    return tok;
}

bool parseU32(const std::string& s, uint32_t& out) {
    try {
        size_t pos = 0;
        unsigned long v = 0;
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            v = std::stoul(s, &pos, 16);
        } else {
            v = std::stoul(s, &pos, 10);
        }
        if (pos != s.size()) return false;
        out = static_cast<uint32_t>(v);
        return true;
    } catch (...) {
        return false;
    }
}

} 

bool Debugger::loadExe(const std::string& path, std::string& errOut) {
    std::vector<uint32_t> code;
    std::vector<int32_t> data;
    uint32_t maxVReg = 0;
    uint32_t entry = kDefaultEntryAddr;
    if (!readExeFile(path, code, data, maxVReg, entry)) {
        errOut = "Failed to load executable: " + path;
        return false;
    }

    mem_ = VmMonitor::buildImage(code, data, maxVReg);
    cpu_ = RiscvCpu(mem_);
    cpu_.setPc(entry);
    entryPc_ = entry;
    codeBase_ = kCodeBase;
    codeEnd_ = codeBase_ + static_cast<uint32_t>(code.size()) * 4u;
    breakpoints_.clear();
    loaded_ = true;

    std::cout << "Loaded " << path << " (" << code.size() << " instructions, entry=0x"
              << std::hex << entry << std::dec << ")\n";
    showState();
    return true;
}

uint32_t Debugger::resolveAddr(uint32_t addr) const {
    if (addr < codeBase_) {
        return codeBase_ + addr * 4u;
    }
    return addr;
}

uint32_t Debugger::insnIndex(uint32_t pc) const {
    if (pc < codeBase_) return 0;
    return (pc - codeBase_) / 4u;
}

void Debugger::showState() const {
    if (!loaded_) {
        std::cout << "(no program loaded)\n";
        return;
    }

    uint32_t pc = cpu_.getPc();
    uint32_t idx = insnIndex(pc);
    uint32_t cur = cpu_.peekInsn();
    uint32_t nextPc = pc + 4;
    uint32_t nextInsn = (nextPc + 4 <= mem_.size()) ? static_cast<uint32_t>(cpu_.readMemWord(nextPc)) : 0u;

    std::cout << "---\n";
    std::cout << "PC=0x" << std::hex << pc << std::dec << "  insn #" << idx;
    if (cpu_.isHalted()) std::cout << "  [HALTED]";
    std::cout << "\n";
    std::cout << "  => " << RiscvCpu::disassemble(cur, pc) << "\n";
    if (!cpu_.isHalted() && nextPc < codeEnd_) {
        std::cout << "  next: " << RiscvCpu::disassemble(nextInsn, nextPc) << "\n";
    }
    std::cout << "call depth: " << cpu_.getCallDepth() << "\n";

    std::cout << "registers:";
    bool any = false;
    for (unsigned r = 0; r < 32; ++r) {
        int32_t v = cpu_.getReg(r);
        if (v != 0) {
            if (!any) std::cout << "\n";
            any = true;
            std::cout << "  x" << r << " = " << v;
            if (r == 2) std::cout << " (sp)";
            if (r == 8) std::cout << " (fp/s0)";
            if (r == 1) std::cout << " (ra)";
            std::cout << "\n";
        }
    }
    if (!any) std::cout << " (all zero)\n";

    if (!breakpoints_.empty()) {
        std::cout << "breakpoints:";
        for (uint32_t bp : breakpoints_) {
            std::cout << " 0x" << std::hex << bp << "(#" << insnIndex(bp) << ")" << std::dec;
        }
        std::cout << "\n";
    }
    std::cout << "---\n";
}

void Debugger::showHelp() const {
    std::cout << "\nDebugger commands:\n";
    std::cout << "  step / si / Enter     Execute next instruction (step into)\n";
    std::cout << "  over / so             Step over function calls\n";
    std::cout << "  out / su              Step out of current function\n";
    std::cout << "  go / c / continue     Run until breakpoint or halt\n";
    std::cout << "  br.add <addr>         Set breakpoint (insn index or 0xPC)\n";
    std::cout << "  br.rem <addr>         Remove breakpoint\n";
    std::cout << "  br / br.list          List breakpoints\n";
    std::cout << "  r<n> / reg <n>        Show register x<n>\n";
    std::cout << "  m<addr> / mem <addr>  Show memory word at address\n";
    std::cout << "  h / help              Show this menu\n";
    std::cout << "  q / quit / exit       Quit debugger\n\n";
}

bool Debugger::atBreakpoint() const {
    return breakpoints_.count(cpu_.getPc()) > 0;
}

bool Debugger::stepInto(std::string& err) {
    if (cpu_.isHalted()) return true;
    CpuStatus s = cpu_.step();
    if (s == CpuStatus::Error) {
        err = cpu_.getLastError();
        return false;
    }
    return true;
}

bool Debugger::stepOver(std::string& err) {
    if (cpu_.isHalted()) return true;
    if (!RiscvCpu::isCallInsn(cpu_.peekInsn())) {
        return stepInto(err);
    }
    int startDepth = cpu_.getCallDepth();
    if (!stepInto(err)) return false;
    if (cpu_.isHalted() || cpu_.getCallDepth() <= startDepth) return true;
    return runUntil(err, startDepth, true);
}

bool Debugger::stepOut(std::string& err) {
    if (cpu_.isHalted()) return true;
    int depth = cpu_.getCallDepth();
    if (depth <= 0) {
        return stepInto(err);
    }
    return runUntil(err, depth - 1, true);
}

bool Debugger::continueRun(std::string& err) {
    if (cpu_.isHalted()) return true;
    return runUntil(err, -1, true);
}

bool Debugger::runUntil(std::string& err, int targetDepth, bool stopOnBp) {
    size_t steps = 0;
    while (!cpu_.isHalted() && steps < kMaxRunSteps) {
        CpuStatus s = cpu_.step();
        ++steps;

        if (s == CpuStatus::Error) {
            err = cpu_.getLastError();
            return false;
        }
        if (s == CpuStatus::Halted) {
            break;
        }

        if (targetDepth >= 0 && cpu_.getCallDepth() <= targetDepth) {
            break;
        }

        if (stopOnBp && atBreakpoint()) {
            break;
        }
    }

    if (steps >= kMaxRunSteps && !cpu_.isHalted()) {
        err = "Step limit reached";
        return false;
    }
    return true;
}

bool Debugger::handleCommand(const std::string& line) {
    std::string err;
    std::string cmd = trim(line);
    if (cmd.empty()) {
        if (!loaded_) {
            std::cout << "No program loaded.\n";
            return true;
        }
        if (!stepInto(err)) {
            std::cerr << "Error: " << err << "\n";
        }
        showState();
        return true;
    }

    std::string lower = toLower(cmd);

    if (lower == "q" || lower == "quit" || lower == "exit") return false;
    if (lower == "h" || lower == "help") {
        showHelp();
        return true;
    }
    if (lower == "step" || lower == "si") {
        if (!stepInto(err)) std::cerr << "Error: " << err << "\n";
        showState();
        return true;
    }
    if (lower == "over" || lower == "so") {
        if (!stepOver(err)) std::cerr << "Error: " << err << "\n";
        showState();
        return true;
    }
    if (lower == "out" || lower == "su") {
        if (!stepOut(err)) std::cerr << "Error: " << err << "\n";
        showState();
        return true;
    }
    if (lower == "go" || lower == "c" || lower == "continue") {
        if (!continueRun(err)) std::cerr << "Error: " << err << "\n";
        showState();
        return true;
    }
    if (lower == "br" || lower == "br.list") {
        if (breakpoints_.empty()) {
            std::cout << "No breakpoints set.\n";
        } else {
            std::cout << "Breakpoints:\n";
            for (uint32_t bp : breakpoints_) {
                std::cout << "  0x" << std::hex << bp << std::dec << " (insn #" << insnIndex(bp) << ")\n";
            }
        }
        return true;
    }

    auto tok = tokenize(cmd);
    if (!tok.empty() && tok[0] == "br.add" && tok.size() >= 2) {
        uint32_t raw = 0;
        if (!parseU32(tok[1], raw)) {
            std::cout << "Invalid address: " << tok[1] << "\n";
            return true;
        }
        uint32_t pc = resolveAddr(raw);
        breakpoints_.insert(pc);
        std::cout << "Breakpoint at 0x" << std::hex << pc << std::dec << " (insn #" << insnIndex(pc) << ")\n";
        return true;
    }
    if (!tok.empty() && tok[0] == "br.rem" && tok.size() >= 2) {
        uint32_t raw = 0;
        if (!parseU32(tok[1], raw)) {
            std::cout << "Invalid address: " << tok[1] << "\n";
            return true;
        }
        uint32_t pc = resolveAddr(raw);
        if (breakpoints_.erase(pc)) {
            std::cout << "Removed breakpoint at 0x" << std::hex << pc << std::dec << "\n";
        } else {
            std::cout << "No breakpoint at 0x" << std::hex << pc << std::dec << "\n";
        }
        return true;
    }

    if (lower.rfind("r", 0) == 0 && lower.size() > 1 && std::isdigit((unsigned char)lower[1])) {
        unsigned r = 0;
        if (parseU32(lower.substr(1), r) && r < 32) {
            std::cout << "x" << r << " = " << cpu_.getReg(r) << "\n";
            return true;
        }
    }
    if (!tok.empty() && (tok[0] == "reg" || tok[0] == "r") && tok.size() >= 2) {
        uint32_t r = 0;
        if (parseU32(tok[1], r) && r < 32) {
            std::cout << "x" << r << " = " << cpu_.getReg(r) << "\n";
            return true;
        }
    }

    if (lower.rfind("m", 0) == 0 && lower.size() > 1 && std::isdigit((unsigned char)lower[1])) {
        uint32_t addr = 0;
        if (parseU32(lower.substr(1), addr)) {
            std::cout << "[0x" << std::hex << addr << std::dec << "] = " << cpu_.readMemWord(addr) << "\n";
            return true;
        }
    }
    if (!tok.empty() && (tok[0] == "mem" || tok[0] == "m") && tok.size() >= 2) {
        uint32_t addr = 0;
        if (parseU32(tok[1], addr)) {
            std::cout << "[0x" << std::hex << addr << std::dec << "] = " << cpu_.readMemWord(addr) << "\n";
            return true;
        }
    }

    if (lower.rfind("load", 0) == 0) {
        auto t2 = tokenize(cmd);
        if (t2.size() >= 3 && t2[1] == "-f") {
            if (!loadExe(t2[2], err)) {
                std::cerr << err << "\n";
            }
            return true;
        }
    }

    std::cout << "Unknown command: " << cmd << " (type 'help')\n";
    return true;
}

void Debugger::runLoop() {
    showHelp();
    std::string line;
    while (true) {
        std::cout << "(dbg) ";
        if (!std::getline(std::cin, line)) break;
        if (!handleCommand(line)) break;
        if (loaded_ && cpu_.isHalted()) {
            std::cout << "Program halted.\n";
        }
    }
    std::cout << "Debugger exited.\n";
}
