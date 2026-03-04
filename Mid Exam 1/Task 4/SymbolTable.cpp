#include "SymbolTable.h"

size_t SymbolTable::addSymbol(const std::string& name, double value) {
    auto it = symbolMap.find(name);
    if (it != symbolMap.end()) {
        symbols[it->second].value = value;
        return it->second;
    }
    
    size_t stringIndex = stringPool.size();
    stringPool.push_back(name);
    
    size_t symbolIndex = symbols.size();
    symbols.push_back({name, value, stringIndex});
    symbolMap[name] = symbolIndex;
    
    return symbolIndex;
}

bool SymbolTable::setValue(const std::string& name, double value) {
    auto it = symbolMap.find(name);
    if (it != symbolMap.end()) {
        symbols[it->second].value = value;
        return true;
    }
    return false;
}

bool SymbolTable::getValue(const std::string& name, double& value) const {
    auto it = symbolMap.find(name);
    if (it != symbolMap.end()) {
        value = symbols[it->second].value;
        return true;
    }
    return false;
}

bool SymbolTable::exists(const std::string& name) const {
    return symbolMap.find(name) != symbolMap.end();
}

void SymbolTable::clear() {
    symbolMap.clear();
    symbols.clear();
    stringPool.clear();
}

size_t SymbolTable::getSymbolCount() const {
    return symbols.size();
}