#include "Deduplicator.hpp"
#include "Hasher.hpp"
#include "core/FsWalker.hpp"
#include <iostream>
#include <unordered_map>

namespace deduplicator {

namespace {
template <typename Algo>
HashTable calcAndGroup(
    const std::unordered_map<std::string, std::vector<fs::path>> &by_size,
    const Algo &algo) {
  HashTable table;
  for (auto &[h, vec] : by_size) {
    for (const fs::path &p : vec) {
      static size_t counter = 0;
      if (++counter % 1000 == 0) {
        std::cerr << "hashed " << counter << " files, current: " << p << '\n';
      }
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
    static int counter = 0;
    if (++counter % 10000 == 0) {
      std::cerr << "Scanned " << counter << " files, current: " << p.string()
                << std::endl;
    }
    uintmax_t sz = static_cast<uintmax_t>(st.st_size);
    by_size[sz].push_back(p);
  });

  std::erase_if(by_size,
                [](const auto &pair) { return pair.second.size() <= 1; });

  std::cerr << "1 Stage: sort by size completed " << '\n';

  constexpr size_t HEAD_BYTES = 4096;
  std::unordered_map<std::string, std::vector<fs::path>> by_head;

  auto hashHead = [&](const fs::path &p) -> std::string {
    if (type == HashType::MD5) {
      return hasher::MD5Algorithm{}.computeHead(p, HEAD_BYTES);
    } else {
      return hasher::SHA256Algorithm{}.computeHead(p, HEAD_BYTES);
    }
  };

  for (auto &[sz, vec] : by_size) {
    for (const auto &p : vec) {
      std::string head = hashHead(p);
      static int counter = 0;
      if (++counter % 10000 == 0) {
        std::cerr << "Hashed " << counter << " files, current: " << p.string()
                  << std::endl;
      }
      if (head.empty())
        continue;
      by_head[head].push_back(p);
    }
  }

  std::erase_if(by_head,
                [](const auto &pair) { return pair.second.size() <= 1; });

  std::cerr << "2 Stage: sort by head completed " << '\n';

  HashTable table;

  if (type == HashType::MD5) {
    table = calcAndGroup(by_head, hasher::MD5Algorithm{});
  } else {
    table = calcAndGroup(by_head, hasher::SHA256Algorithm{});
  }

  std::erase_if(table,
                [](const auto &pair) { return pair.second.size() <= 1; });
  std::cerr << "3 Stage: sort by full hash completed " << '\n';

  return table;
}
} // namespace deduplicator
