#include "Deduplicator.hpp"
#include "FsWalker.hpp"
#include "Log.hpp"
#include "ThreadPool.hpp"
#include "cli/Style.hpp"
#include "cli/TtyLine.hpp"
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace deduplicator {

namespace {

template <typename Cont> size_t countFiles(const Cont &cont) {
  size_t total = 0;
  for (const auto &[f, vec] : cont) {
    total += vec.size();
  }
  return total;
}

template <typename Key>
std::vector<std::pair<Key, fs::path>>
toVector(const std::unordered_map<Key, std::vector<fs::path>> &map) {
  std::vector<std::pair<Key, fs::path>> vec;

  for (const auto &[key, v] : map) {
    for (const auto &p : v) {
      vec.emplace_back(key, p);
    }
  }

  return vec;
}

template <typename Algo>
HashTable calcAndGroup(const HashTable &by_head, const Algo &algo) {
  LOG_DEBUG("calcAndGroup started");
  std::vector<std::pair<std::string, fs::path>> by_head_vec;
  {
    TIME_SCOPE("create simple vector [head_hash, file]");
    by_head_vec = toVector(by_head);
  }
  HashTable table;
  std::mutex tm;

  std::atomic<size_t> cnt{0};

  {
    TIME_SCOPE("sort by full hash");
    threadpool::parallel_for_each(
        by_head_vec, [&algo, &table, &cnt, &tm](const auto &pair) {
          const auto &[head_hash, file] = pair;
          std::string hash = hasher::getHash(file, algo);
          if (hash.empty()) {
            return;
          }
          std::lock_guard<std::mutex> lock(tm);
          table[hash].push_back(file);
          ++cnt;
        });
  }

  LOG_DEBUG("Files with same full hash: " << cnt << "\n");

  return table;
}
} // namespace

HashTable findDuplicates(const fs::path &path, HashType type) {
  LOG_DEBUG("findDuplicated started");
  tty_line::Cursor cursor_guard{};

  std::unordered_map<uintmax_t, std::vector<fs::path>> by_size;
  size_t files_cnt = 0;

  {
    TIME_SCOPE("walk + group by size");
    fswalker::walk(
        path, [&by_size, &files_cnt](const fs::path &p, const struct stat &st) {
          if (st.st_size == 0) {
            return;
          }
          tty_line::updateLine("Scanning: ", p.c_str());
          uintmax_t sz = static_cast<uintmax_t>(st.st_size);
          by_size[sz].push_back(p);
          ++files_cnt;
        });
    std::string fin = style::colored(
        "Scanned: " + std::to_string(files_cnt) + " files", style::green());

    tty_line::finish(fin);
  }

  {
    TIME_SCOPE("erase singletons by size");
    std::erase_if(by_size,
                  [](const auto &pair) { return pair.second.size() <= 1; });
  }

  LOG_DEBUG("Files scanned: " << files_cnt << "\nDuplicates by size (1 stage): "
                              << countFiles(by_size)
                              << " -> Groups formed: " << by_size.size());

  std::vector<std::pair<uintmax_t, fs::path>> all_files;
  {
    TIME_SCOPE("create simple vector [size, file]");
    all_files = toVector(by_size);
  }

  std::unordered_map<uintmax_t, std::vector<fs::path>>().swap(by_size);

  constexpr size_t HEAD_BYTES = 512;

  auto hashHead = [&](const fs::path &p) -> std::string {
    if (type == HashType::MD5) {
      return hasher::MD5Algorithm{}.computeHead(p, HEAD_BYTES);
    } else {
      return hasher::SHA256Algorithm{}.computeHead(p, HEAD_BYTES);
    }
  };

  std::atomic<size_t> head_files_cnt{0};

  std::unordered_map<std::string, std::vector<fs::path>> by_head;
  std::mutex hm;

  {
    TIME_SCOPE("sort by hash head");

    size_t total = all_files.size();
    tty_line::ProgressBar bar(total);
    std::atomic<bool> done{false};

    threadpool::ThreadPool pool(1);
    pool.submit([&bar, &done, &head_files_cnt]() {
      while (!done.load()) {
        bar.updateBar("HeadHash: ", head_files_cnt.load());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    });

    threadpool::parallel_for_each(
        all_files,
        [&hashHead, &by_head, &head_files_cnt, &hm](const auto &pair) {
          const auto &[sz, file] = pair;
          std::string head = hashHead(file);
          if (head.empty())
            return;

          {
            std::lock_guard<std::mutex> lock(hm);
            by_head[head].push_back(file);
          }
          head_files_cnt++;
        });

    done.store(true);

    std::string fin =
        "Head hashed: " + std::to_string(head_files_cnt) + " files";
    bar.finishProgress(fin);
  }

  {
    TIME_SCOPE("erase singletons by heads");
    std::erase_if(by_head,
                  [](const auto &pair) { return pair.second.size() <= 1; });
  }

  LOG_DEBUG("Files head hashed: "
            << head_files_cnt << "\nDuplicates by head_hash (2 stage):"
            << countFiles(by_head) << " -> Groups formed: " << by_head.size());

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

  LOG_DEBUG("Duplicates by full hash (3 stage):"
            << countFiles(table) << " -> Groups formed: " << table.size());

  return table;
}
} // namespace deduplicator
