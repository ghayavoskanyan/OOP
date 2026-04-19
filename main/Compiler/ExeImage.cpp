#include "ExeImage.h"
#include <fstream>

bool writeExeFile(const std::string& path, const std::vector<uint32_t>& code, const std::vector<int32_t>& data,
                  uint32_t maxVReg, uint32_t entryByteAddr) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;

    ExeHeader h{};
    h.magic = kExeMagic;
    h.version = kExeVersion;
    h.codeWords = static_cast<uint32_t>(code.size());
    h.dataWords = static_cast<uint32_t>(data.size());
    h.entryByteAddr = entryByteAddr;
    h.maxVReg = maxVReg;

    f.write(reinterpret_cast<const char*>(&h), sizeof(h));
    if (!f) return false;

    for (uint32_t w : code) {
        f.write(reinterpret_cast<const char*>(&w), sizeof(w));
    }
    for (int32_t v : data) {
        f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }
    return (bool)f;
}

bool readExeFile(const std::string& path, std::vector<uint32_t>& code, std::vector<int32_t>& data, uint32_t& maxVReg,
                 uint32_t& entryByteAddr) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    ExeHeader h{};
    f.read(reinterpret_cast<char*>(&h), sizeof(h));
    if (!f || h.magic != kExeMagic) return false;

    code.resize(h.codeWords);
    data.resize(h.dataWords);
    maxVReg = h.maxVReg;
    entryByteAddr = h.entryByteAddr;

    for (uint32_t i = 0; i < h.codeWords; ++i) {
        f.read(reinterpret_cast<char*>(&code[i]), sizeof(uint32_t));
    }
    for (uint32_t i = 0; i < h.dataWords; ++i) {
        f.read(reinterpret_cast<char*>(&data[i]), sizeof(int32_t));
    }
    return (bool)f;
}
