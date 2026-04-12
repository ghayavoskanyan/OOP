#include "SymbolTable.h"

size_t SymbolTable::getIndex(const std::string& name) {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) {
        size_t idx = values.size();
        nameToIndex[name] = idx;
        values.push_back(0.0);
        return idx;
    }
    return it->second;
}

bool SymbolTable::getValue(const std::string& name, double& value) {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) return false;
    value = values[it->second];
    return true;
}

double SymbolTable::getValueByIndex(size_t idx) const {
    if (idx < values.size()) return values[idx];
    return 0.0;
}

void SymbolTable::setValueByIndex(size_t idx, double val) {
    if (idx < values.size()) values[idx] = val;
}

std::vector<double>& SymbolTable::getValuesVector() {
    return values;
}