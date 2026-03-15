#include "SymbolTable.h"
#include <algorithm>

std::unordered_map<std::string, size_t> SymbolTable::nameToId;
std::vector<std::string> SymbolTable::stringPool;
std::vector<SymbolTable::SymbolInfo> SymbolTable::symbols;

std::string SymbolTable::trim(const std::string& name) const {
    std::string s = name;
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

size_t SymbolTable::addSymbol(const std::string& name, double value) {
    std::string cleanName = trim(name);
    auto it = nameToId.find(cleanName);
    if (it != nameToId.end()) {
        symbols[it->second].value = value;
        return it->second;
    }
    size_t newStrIdx = stringPool.size();
    stringPool.push_back(cleanName);
    size_t newSymIdx = symbols.size();
    symbols.push_back({newStrIdx, value});
    nameToId[cleanName] = newSymIdx;
    return newSymIdx;
}

bool SymbolTable::setValue(const std::string& name, double value) {
    std::string cleanName = trim(name);
    auto it = nameToId.find(cleanName);
    if (it != nameToId.end()) {
        symbols[it->second].value = value;
        return true;
    }
    return false;
}

bool SymbolTable::getValue(const std::string& name, double& value) const {
    std::string cleanName = trim(name);
    auto it = nameToId.find(cleanName);
    if (it != nameToId.end()) {
        value = symbols[it->second].value;
        return true;
    }
    return false;
}

bool SymbolTable::exists(const std::string& name) const {
    return nameToId.find(trim(name)) != nameToId.end();
}

size_t SymbolTable::getSymbolCount() const { 
    return symbols.size();
}

void SymbolTable::clear() {
    nameToId.clear();
    symbols.clear();
    stringPool.clear();
}