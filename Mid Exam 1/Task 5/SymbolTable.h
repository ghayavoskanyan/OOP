#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable {
private:
    struct SymbolInfo {
        size_t stringIndex; 
        double value;
    };

    static std::unordered_map<std::string, size_t> nameToId; 
    static std::vector<std::string> stringPool;
    static std::vector<SymbolInfo> symbols; 

    std::string trim(const std::string& name) const;

public:
    size_t addSymbol(const std::string& name, double value = 0.0);
    bool setValue(const std::string& name, double value);
    bool getValue(const std::string& name, double& value) const;
    bool exists(const std::string& name) const;
    size_t getSymbolCount() const;
    void clear();
};