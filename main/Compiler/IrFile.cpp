#include "IrFile.h"
#include <fstream>
#include <variant>

namespace {

constexpr uint32_t kIrMagic = 0x52494F50u;
constexpr uint32_t kIrVersion = 2u;

void writeU32(std::ostream& o, uint32_t v) { o.write(reinterpret_cast<const char*>(&v), 4); }

void writeU8(std::ostream& o, uint8_t v) { o.write(reinterpret_cast<const char*>(&v), 1); }

bool readU32(std::istream& i, uint32_t& v) {
    i.read(reinterpret_cast<char*>(&v), 4);
    return (bool)i;
}

} // namespace

bool writeIrFile(const std::string& path, const std::vector<Instruction>& program) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    writeU32(f, kIrMagic);
    writeU32(f, kIrVersion);
    writeU32(f, static_cast<uint32_t>(program.size()));
    for (const auto& ins : program) {
        writeU8(f, static_cast<uint8_t>(ins.opcode));
        if (ins.opcode == OpCode::LI || ins.opcode == OpCode::LIL || ins.opcode == OpCode::LIH) {
            writeU8(f, 1);
            auto& d = std::get<LiData>(ins.data);
            writeU32(f, d.dest);
            writeU32(f, d.value);
        } else {
            writeU8(f, 0);
            auto& d = std::get<ArithData>(ins.data);
            writeU32(f, d.dest);
            writeU32(f, d.left);
            writeU32(f, d.right);
        }
    }
    return (bool)f;
}

bool readIrFile(const std::string& path, std::vector<Instruction>& program) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    uint32_t magic = 0, ver = 0, count = 0;
    if (!readU32(f, magic) || magic != kIrMagic) return false;
    if (!readU32(f, ver) || (ver != 1u && ver != 2u)) return false;
    if (!readU32(f, count)) return false;
    program.clear();
    program.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        uint8_t op = 0, tag = 0;
        f.read(reinterpret_cast<char*>(&op), 1);
        f.read(reinterpret_cast<char*>(&tag), 1);
        if (!f) return false;
        auto opcode = static_cast<OpCode>(op);
        if (tag == 1) {
            uint32_t dest = 0, val = 0;
            if (!readU32(f, dest)) return false;
            if (!readU32(f, val)) return false;
            program.emplace_back(opcode, LiData{dest, val});
        } else {
            uint32_t a = 0, b = 0, c = 0;
            if (!readU32(f, a) || !readU32(f, b) || !readU32(f, c)) return false;
            program.emplace_back(opcode, ArithData{a, b, c});
        }
    }
    return true;
}
