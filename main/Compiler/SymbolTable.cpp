#include "SymbolTable.h"

SymbolTable::SymbolTable() {}

size_t SymbolTable::getIndex(const std::string& name) {
    auto it = nameToIndex.find(name);
    if (it != nameToIndex.end()) {
        return it->second;
    }
    size_t idx = values.size();
    nameToIndex[name] = idx;
    values.push_back(0);
    return idx;
}

bool SymbolTable::hasSymbol(const std::string& name) const {
    return nameToIndex.find(name) != nameToIndex.end();
}

void SymbolTable::addSymbol(const std::string& name) {
    if (!hasSymbol(name)) {
        size_t idx = values.size();
        nameToIndex[name] = idx;
        values.push_back(0);
    }
}

bool SymbolTable::getValue(const std::string& name, int32_t& out) const {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) return false;
    size_t idx = it->second;
    if (idx >= values.size()) return false;
    out = values[idx];
    return true;
}

bool SymbolTable::setValue(const std::string& name, int32_t val) {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) return false;
    size_t idx = it->second;
    if (idx >= values.size()) values.resize(idx + 1, 0);
    values[idx] = val;
    return true;
}

std::vector<int32_t>& SymbolTable::getValuesVector() { return values; }

const std::vector<int32_t>& SymbolTable::getValuesVector() const { return values; }

int32_t SymbolTable::getValue(size_t idx) const {
    return (idx < values.size()) ? values[idx] : 0;
}

void SymbolTable::setValue(size_t idx, int32_t val) {
    if (idx >= values.size()) values.resize(idx + 1, 0);
    values[idx] = val;
}
