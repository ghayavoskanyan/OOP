#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

class StaticStorage {
public:
    int32_t get(const std::string& key) const;
    void set(const std::string& key, int32_t v);
    bool contains(const std::string& key) const;

private:
    std::unordered_map<std::string, int32_t> cells_;
};
