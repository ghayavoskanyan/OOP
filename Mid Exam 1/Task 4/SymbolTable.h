#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class SymbolTable {
private:
    struct SymbolInfo {
        std::string name;
        double value;
        size_t stringIndex;
    };
    
    std::unordered_map<std::string, size_t> symbolMap;
    std::vector<SymbolInfo> symbols;
    std::vector<std::string> stringPool;
    
public:
    size_t addSymbol(const std::string& name, double value = 0.0);
    bool setValue(const std::string& name, double value);
    bool getValue(const std::string& name, double& value) const;
    bool exists(const std::string& name) const;
    void clear();
    size_t getSymbolCount() const;
};