#pragma once
#include <string>
#include <vector>

namespace linker {

bool linkObjectFiles(const std::vector<std::string>& objectPaths, const std::string& outExePath);

}
