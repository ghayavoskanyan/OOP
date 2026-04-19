#pragma once
#include "VM.h"
#include <string>
#include <vector>

bool writeIrFile(const std::string& path, const std::vector<Instruction>& program);
bool readIrFile(const std::string& path, std::vector<Instruction>& program);
