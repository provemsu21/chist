#include "Deduplicator.hpp"
#include "Hasher.hpp"
#include "core/FsWalker.hpp"
#include <unordered_map>

namespace deduplicator {

namespace {
template <typename Algo>
HashTable calcAndGroup(
    const std::unordered_map<uintmax_t, std::vector<fs::path>> &by_size,
    Algo algo) {
  HashTable table;
  for (auto &[sz, vec] : by_size) {
    for (const fs::path &p : vec) {
      std::string hash = hasher::getHash(p, algo);
      if (hash.empty()) {
        continue;
      }
      table[hash].push_back(p);
    }
  }

  return table;
}
} // namespace

HashTable findDuplicates(const fs::path &path, HashType type) {
  std::unordered_map<uintmax_t, std::vector<fs::path>> by_size;

  fswalker::walk(path, [&by_size](const fs::path &p, const struct stat &st) {
    if (st.st_size == 0) {
      return;
    }
    uintmax_t sz = static_cast<uintmax_t>(st.st_size);
    by_size[sz].push_back(p);
  });

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
} // namespace deduplicator
