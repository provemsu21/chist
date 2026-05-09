#include "Deduplicator.hpp"
#include "Hasher.hpp"
#include <filesystem>
#include <unordered_map>

using namespace deduplicator;

namespace {
template <typename Algo>
HashTable calcAndGroup(
    const std::unordered_map<uintmax_t, std::vector<fs::path>> &by_size,
    Algo algo) {
  HashTable table;
  for (auto &[sz, vec] : by_size) {
    for (const fs::path &p : vec) {
      std::string hash = hasher::getHash(p, algo);
      table[hash].push_back(p);
    }
  }

  return table;
}
} // namespace

HashTable findDuplicates(const fs::path &path, HashType type) {
  std::unordered_map<uintmax_t, std::vector<fs::path>> by_size;

  for (const fs::directory_entry &dir_entry :
       fs::recursive_directory_iterator(path)) {
    if (!dir_entry.is_regular_file())
      continue;
    fs::path filepath = dir_entry.path();
    uintmax_t filesize = fs::file_size(filepath);
    by_size[filesize].push_back(filepath);
  }

  std::erase_if(by_size,
                [](const auto &pair) { return pair.second.size() <= 1; });

  HashTable table;

  if (type == HashType::MD5) {
    table = calcAndGroup(by_size, hasher::MD5Algorithm{});
  } else {
    table = calcAndGroup(by_size, hasher::SHA256Algorithm{});
  }

  std::erase_if(table,
                [](const auto &pair) { return pair.second.size() <= 1; });

  return table;
}
