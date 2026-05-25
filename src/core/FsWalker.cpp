#include "CacheDetector.hpp"
#include "FsWalker.hpp"
#include "cli/TtyLine.hpp"
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <string_view>
#include <sys/stat.h>
#include <system_error>
#include <unordered_set>

namespace fswalker {

namespace {
namespace fs = std::filesystem;

struct FileId {
  dev_t dev;
  ino_t ino;
  bool operator==(const FileId &) const = default;
};

template <typename T>
inline void hash_combine(std::size_t &seed, const T &v) noexcept {
  seed ^= std::hash<T>{}(v) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

struct FileHash {
  size_t operator()(const FileId &id) const noexcept {
    std::size_t seed = 0;
    hash_combine(seed, id.dev);
    hash_combine(seed, id.ino);
    return seed;
  }
};

bool isInBlocked(const fs::path &p) {
  static const std::vector<std::string_view> blocked = {
      "/System/Volumes",
      "/private/var/vm",
      "/dev",
      "/.vol",
      "/Volumes",
      "/.Spotlight-V100",
      "/.fseventsd",
      "/.DocumentRevisions-V100",
      "/.TemporaryItems",
      "/.Trashes",
  };

  const std::string s = p.string();
  for (std::string_view b : blocked) {
    if (s == b || s.starts_with(std::string(b) + "/")) {
      return true;
    }
  }

  return cache_detector::isCache(p);
}

void walkImpl(const fs::path &dir, const fswalker::Visitor &visitor,
              std::unordered_set<FileId, FileHash> &seen) {
  if (isInBlocked(dir))
    return;
  std::error_code ec;
  auto it = fs::directory_iterator(
      dir, fs::directory_options::skip_permission_denied, ec);
  if (ec)
    return;
  auto end = fs::directory_iterator{};
  while (it != end) {
    const fs::path entry_path = it->path();
    struct stat st {};
    if (lstat(entry_path.c_str(), &st) == 0) {
      const FileId fi = {st.st_dev, st.st_ino};
      if (S_ISREG(st.st_mode)) {
        auto [sit, inserted] = seen.insert(fi);
        if (inserted)
          visitor(entry_path, st);
      } else if (S_ISDIR(st.st_mode)) {
        auto [sit, inserted] = seen.insert(fi);
        if (inserted) {
          walkImpl(entry_path, visitor, seen);
        }
      }
    }
    it.increment(ec);
    if (ec)
      return;
  }
}

} // namespace

void walk(const fs::path &p, const Visitor &visitor) {
  std::unordered_set<FileId, FileHash> seen;
  walkImpl(p, visitor, seen);
}

} // namespace fswalker
