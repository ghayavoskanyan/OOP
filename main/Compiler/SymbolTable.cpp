#include "SymbolTable.h"

SymbolTable::SymbolTable() {}

size_t SymbolTable::getIndex(const std::string& name) {
    auto it = nameToIndex.find(name);
    if (it != nameToIndex.end()) {
        return it->second;
    }
    size_t idx = values.size();
    nameToIndex[name] = idx;
    values.push_back(0.0);
    return idx;
}

bool SymbolTable::hasSymbol(const std::string& name) const {
    return nameToIndex.find(name) != nameToIndex.end();
}

void SymbolTable::addSymbol(const std::string& name) {
    if (!hasSymbol(name)) {
        size_t idx = values.size();
        nameToIndex[name] = idx;
        values.push_back(0.0);
    }
}

bool SymbolTable::getValue(const std::string& name, double& out) const {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) return false;
    size_t idx = it->second;
    if (idx >= values.size()) return false;
    out = values[idx];
    return true;
}

bool SymbolTable::setValue(const std::string& name, double val) {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) return false;
    size_t idx = it->second;
    if (idx >= values.size()) values.resize(idx + 1, 0.0);
    values[idx] = val;
    return true;
}

std::vector<double>& SymbolTable::getValuesVector() {
    return values;
}

const std::vector<double>& SymbolTable::getValuesVector() const {
    return values;
}

double SymbolTable::getValue(size_t idx) const {
    return (idx < values.size()) ? values[idx] : 0.0;
}

void SymbolTable::setValue(size_t idx, double val) {
    if (idx >= values.size()) values.resize(idx + 1, 0.0);
    values[idx] = val;
}