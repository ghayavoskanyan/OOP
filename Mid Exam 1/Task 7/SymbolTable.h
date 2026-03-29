/*SymbolTable.h*/ #pragma once
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable {
private:
    std::unordered_map<std::string, size_t> nameToIndex;
    std::vector<double> values;

public:
    bool getValue(const std::string& name, double& value);
    size_t getIndex(const std::string& name);
    double getValueByIndex(size_t idx) const;
    void setValueByIndex(size_t idx, double val);
    std::vector<double>& getValuesVector();
};