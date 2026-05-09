#ifndef CORE_DEDUPLICATOR_HPP_
#define CORE_DEDUPLICATOR_HPP_

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace deduplicator {

namespace fs = std::filesystem;
using HashTable = std::unordered_map<std::string, std::vector<fs::path>>;

enum class HashType { MD5, SHA256 };
HashTable findDuplicates(const fs::path &path, HashType type);

} // namespace deduplicator

#endif // CORE_DEDUPLICATOR_HPP_
