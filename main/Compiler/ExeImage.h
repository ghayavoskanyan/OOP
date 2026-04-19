#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct ExeHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t codeWords;
    uint32_t dataWords;
    uint32_t entryByteAddr;
    uint32_t maxVReg;
};

constexpr uint32_t kExeMagic = 0xC0FFEE01u;
constexpr uint32_t kExeVersion = 1u;
constexpr uint32_t kDefaultEntryAddr = 0x1000u;

bool writeExeFile(const std::string& path, const std::vector<uint32_t>& code, const std::vector<int32_t>& data,
                  uint32_t maxVReg, uint32_t entryByteAddr = kDefaultEntryAddr);

bool readExeFile(const std::string& path, std::vector<uint32_t>& code, std::vector<int32_t>& data, uint32_t& maxVReg,
                 uint32_t& entryByteAddr);
