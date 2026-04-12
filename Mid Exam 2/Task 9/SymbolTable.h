#pragma once
#include <string>
#include <vector>
#include <unordered_map>

class SymbolTable {
private:
    std::unordered_map<std::string, size_t> nameToIndex;
    std::vector<double> values;
public:
    SymbolTable();                                   // defined in .cpp
    ~SymbolTable() = default;

    // Returns index, creates if not exists
    size_t getIndex(const std::string& name);

    // Check existence
    bool hasSymbol(const std::string& name) const;
    void addSymbol(const std::string& name);

    // Get value by name (returns true if found)
    bool getValue(const std::string& name, double& out) const;
    // Set value by name
    bool setValue(const std::string& name, double val);

    // For VM access
    std::vector<double>& getValuesVector();
    const std::vector<double>& getValuesVector() const;

    // Index-based access
    double getValue(size_t idx) const;
    void setValue(size_t idx, double val);
};