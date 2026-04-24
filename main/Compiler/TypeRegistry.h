#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

enum class AggregateKind { Struct, Union, Class };

struct FieldInfo {
    std::string name;
    int32_t offset{0};
    bool isPublic{true};
};

struct AggregateType {
    AggregateKind kind{AggregateKind::Struct};
    std::vector<FieldInfo> fields;
    int32_t sizeWords{0};
};

class TypeRegistry {
public:
    void addEnumConst(const std::string& name, int32_t value);
    bool hasEnumConst(const std::string& name) const;
    int32_t getEnumConst(const std::string& name) const;

    void addAggregate(const std::string& typeName, AggregateKind k, std::vector<FieldInfo> fields);
    const AggregateType* findAggregate(const std::string& name) const;

    void declareInstance(const std::string& varName, const std::string& typeName);
    const std::string* findInstanceType(const std::string& varName) const;
    std::string resolveStorageName(const std::string& name) const;

private:
    std::unordered_map<std::string, int32_t> enumConstants_;
    std::unordered_map<std::string, AggregateType> aggregates_;
    std::unordered_map<std::string, std::string> instanceTypes_;
};
