#include "StaticStorage.h"

int32_t StaticStorage::get(const std::string& key) const {
    auto it = cells_.find(key);
    return it == cells_.end() ? 0 : it->second;
}

void StaticStorage::set(const std::string& key, int32_t v) { cells_[key] = v; }

bool StaticStorage::contains(const std::string& key) const { return cells_.find(key) != cells_.end(); }
