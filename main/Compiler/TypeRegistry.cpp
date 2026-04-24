#include "TypeRegistry.h"
#include <algorithm>

void TypeRegistry::addEnumConst(const std::string& name, int32_t value) { enumConstants_[name] = value; }

bool TypeRegistry::hasEnumConst(const std::string& name) const {
    return enumConstants_.find(name) != enumConstants_.end();
}

int32_t TypeRegistry::getEnumConst(const std::string& name) const {
    auto it = enumConstants_.find(name);
    return it == enumConstants_.end() ? 0 : it->second;
}

void TypeRegistry::addAggregate(const std::string& typeName, AggregateKind k, std::vector<FieldInfo> fields) {
    AggregateType t;
    t.kind = k;
    t.fields = std::move(fields);
    int32_t off = 0;
    int32_t maxSz = 0;
    if (k == AggregateKind::Union) {
        for (auto& f : t.fields) {
            f.offset = 0;
            maxSz = std::max(maxSz, 1);
        }
        t.sizeWords = maxSz;
    } else {
        for (auto& f : t.fields) {
            f.offset = off;
            off += 1;
        }
        t.sizeWords = off;
    }
    aggregates_[typeName] = std::move(t);
}

const AggregateType* TypeRegistry::findAggregate(const std::string& name) const {
    auto it = aggregates_.find(name);
    return it == aggregates_.end() ? nullptr : &it->second;
}

void TypeRegistry::declareInstance(const std::string& varName, const std::string& typeName) {
    instanceTypes_[varName] = typeName;
}

const std::string* TypeRegistry::findInstanceType(const std::string& varName) const {
    auto it = instanceTypes_.find(varName);
    if (it == instanceTypes_.end()) return nullptr;
    return &it->second;
}

std::string TypeRegistry::resolveStorageName(const std::string& name) const {
    size_t dot = name.find('.');
    if (dot == std::string::npos) return name;

    std::string base = name.substr(0, dot);
    std::string field = name.substr(dot + 1);
    (void)field;
    auto itInst = instanceTypes_.find(base);
    if (itInst == instanceTypes_.end()) return name;
    auto itAggr = aggregates_.find(itInst->second);
    if (itAggr == aggregates_.end()) return name;
    const AggregateType& t = itAggr->second;
    if (t.kind != AggregateKind::Union) return name;
    if (t.fields.empty()) return name;
    return base + "." + t.fields.front().name;
}
