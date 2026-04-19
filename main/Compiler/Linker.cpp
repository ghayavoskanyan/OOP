#include "Linker.h"
#include "ExeImage.h"
#include <fstream>
#include <iostream>

namespace linker {

bool linkObjectFiles(const std::vector<std::string>& objectPaths, const std::string& outExePath) {
    if (objectPaths.empty()) {
        std::cerr << "linker: no input object files.\n";
        return false;
    }

    std::vector<uint32_t> mergedCode;
    std::vector<int32_t> mergedData;
    uint32_t maxVReg = 0;
    uint32_t entry = kDefaultEntryAddr;

    for (const auto& path : objectPaths) {
        std::vector<uint32_t> code;
        std::vector<int32_t> data;
        uint32_t mv = 0;
        uint32_t ent = entry;
        if (!readExeFile(path, code, data, mv, ent)) {
            std::cerr << "linker: failed to read " << path << "\n";
            return false;
        }
        if (mergedCode.empty()) {
            entry = ent;
            mergedCode = std::move(code);
            mergedData = std::move(data);
            maxVReg = mv;
        } else {
            for (uint32_t w : code) mergedCode.push_back(w);
            for (int32_t v : data) mergedData.push_back(v);
            if (mv > maxVReg) maxVReg = mv;
        }
    }

    return writeExeFile(outExePath, mergedCode, mergedData, maxVReg, entry);
}

}
