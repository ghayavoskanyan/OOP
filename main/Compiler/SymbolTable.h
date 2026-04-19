#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolTable {
private:
    std::unordered_map<std::string, size_t> nameToIndex;
    std::vector<int32_t> values;

public:
    SymbolTable();
    ~SymbolTable() = default;

    size_t getIndex(const std::string& name);

    bool hasSymbol(const std::string& name) const;
    void addSymbol(const std::string& name);

    bool getValue(const std::string& name, int32_t& out) const;
    bool setValue(const std::string& name, int32_t val);

    std::vector<int32_t>& getValuesVector();
    const std::vector<int32_t>& getValuesVector() const;

    int32_t getValue(size_t idx) const;
    void setValue(size_t idx, int32_t val);
};
