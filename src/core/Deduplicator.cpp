#include "Deduplicator.hpp"
#include "FsWalker.hpp"
#include "Hasher.hpp"
#include "Log.hpp"
#include <unordered_map>

namespace deduplicator {

namespace {
template <typename Algo>
HashTable calcAndGroup(
    const std::unordered_map<std::string, std::vector<fs::path>> &by_head,
    const Algo &algo) {
  HashTable table;

  size_t cnt = 0;
  for (auto &[h, vec] : by_head) {
    for (const fs::path &p : vec) {

      if (++cnt % 1000 == 0) {
        LOG_DEBUG("Hashed " << cnt << " files, current: " << p.string());
      }
      std::string hash = hasher::getHash(p, algo);
      if (hash.empty()) {
        continue;
      }
      table[hash].push_back(p);
    }
  }

  LOG_DEBUG("Files full hashed: " << cnt
                                  << " -> Groups formed: " << table.size());

  return table;
}
} // namespace

HashTable findDuplicates(const fs::path &path, HashType type) {
  LOG_DEBUG("findDuplicated started");

  std::unordered_map<uintmax_t, std::vector<fs::path>> by_size;
  size_t files_cnt = 0;

  {
    TIME_SCOPE("walk + group by size");
    fswalker::walk(path, [&by_size, &files_cnt](const fs::path &p,
                                                const struct stat &st) {
      if (st.st_size == 0) {
        return;
      }
      if (++files_cnt % 100000 == 0) {
        LOG_DEBUG("Scanned " << files_cnt << " files, current: " << p.string());
      }
      uintmax_t sz = static_cast<uintmax_t>(st.st_size);
      by_size[sz].push_back(p);
    });
  }

  {
    TIME_SCOPE("erase singletons by size");
    std::erase_if(by_size,
                  [](const auto &pair) { return pair.second.size() <= 1; });
  }

  LOG_DEBUG("Files scanned: " << files_cnt
                              << " -> Groups formed: " << by_size.size());

  constexpr size_t HEAD_BYTES = 4096;
  std::unordered_map<std::string, std::vector<fs::path>> by_head;

  auto hashHead = [&](const fs::path &p) -> std::string {
    if (type == HashType::MD5) {
      return hasher::MD5Algorithm{}.computeHead(p, HEAD_BYTES);
    } else {
      return hasher::SHA256Algorithm{}.computeHead(p, HEAD_BYTES);
    }
  };

  size_t head_files_cnt = 0;

  {
    TIME_SCOPE("sort by hash head");
    for (auto &[sz, vec] : by_size) {
      for (const auto &p : vec) {

        std::string head = hashHead(p);
        if (head.empty())
          continue;
        if (++head_files_cnt % 10000 == 0) {
          LOG_DEBUG("Hashed " << head_files_cnt
                              << " files, current: " << p.string());
        }
        by_head[head].push_back(p);
      }
    }
  }

  {
    TIME_SCOPE("erase singletons by heads");
    std::erase_if(by_head,
                  [](const auto &pair) { return pair.second.size() <= 1; });
  }

  LOG_DEBUG("Files head hashed: " << head_files_cnt
                                  << " -> Groups formed: " << by_head.size());

  HashTable table;

  if (type == HashType::MD5) {
    table = calcAndGroup(by_head, hasher::MD5Algorithm{});
  } else {
    table = calcAndGroup(by_head, hasher::SHA256Algorithm{});
  }

  {
    TIME_SCOPE("erase singletons by full hash");
    std::erase_if(table,
                  [](const auto &pair) { return pair.second.size() <= 1; });
  }
  return table;
}
} // namespace deduplicator
