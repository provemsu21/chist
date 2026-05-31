#ifndef CORE_DEDUPLICATOR_HPP_
#define CORE_DEDUPLICATOR_HPP_

#include "Hasher.hpp"
#include "cli/Progress.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace deduplicator {

class IProgress;

namespace fs = std::filesystem;
using HashTable = std::unordered_map<std::string, std::vector<fs::path>>;

HashTable findDuplicates(const fs::path &path, hasher::HashType type,
                         progress::IProgress *prog = nullptr);

} // namespace deduplicator

#endif // CORE_DEDUPLICATOR_HPP_
