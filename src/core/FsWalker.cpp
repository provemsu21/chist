#include "FsWalker.hpp"
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
  static const std::vector<fs::path> user_blocked = [] {
    const char *home = getenv("HOME");
    if (!home)
      return std::vector<fs::path>{};
    const fs::path h = fs::path(home);
    return std::vector<fs::path>{
        h / "Library" / "Caches",
        h / "Library" / "Containers",
        h / "Library" / "Application Support" / "CrashReporter",
        h / "Library" / "Logs",
        h / "Library" / "Saved Application State",
        h / "Library" / "WebKit",
        h / "Library" / "HTTPStorages",
        h / "Library" / "Cookies",

        h / "Library" / "Developer" / "Xcode" / "DerivedData",
        h / "Library" / "Developer" / "CoreSimulator" / "Caches",
        h / "Library" / "Application Support" / "Code" / "Cache",
        h / "Library" / "Application Support" / "Code" / "CachedData",
        h / "Library" / "Application Support" / "Code" / "logs",
        h / "Library" / "Application Support" / "Google" / "Chrome" /
            "Default" / "Cache",
        h / "Library" / "Application Support" / "Firefox" / "Profiles",
        h / "Library" / "Application Support" / "JetBrains" / "Toolbox" /
            "logs",

        h / ".Trash",

        h / ".npm",
        h / ".yarn" / "cache",
        h / ".cache",
        h / ".gradle" / "caches",
        h / ".cargo" / "registry",
        h / "Library" / "pnpm" / "store",
        h / "go" / "pkg" / "mod" / "cache",

        h / ".pyenv" / "cache",
        h / ".nvm" / ".cache",
    };
  }();

  const std::string s = p.string();
  for (std::string_view b : blocked) {
    if (s == b || s.starts_with(std::string(b) + "/")) {
      return true;
    }
  }

  for (const fs::path &ub : user_blocked) {
    const std::string bs = ub.string();
    if (s == bs || s.starts_with(bs + "/"))
      return true;
  }
  return false;
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
