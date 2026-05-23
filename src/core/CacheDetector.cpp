#include "CacheDetector.hpp"

#include <array>
#include <fcntl.h>
#include <string_view>
#include <sys/stat.h>
#include <unistd.h>

namespace cache_detector {
namespace {

bool hasCacheLikeName(const fs::path &dir) {
  static constexpr std::array names = {
      std::string_view{"node_modules"}, std::string_view{"bower_components"},
      std::string_view{".pnpm"},

      std::string_view{".git"},         std::string_view{".svn"},
      std::string_view{".hg"},

      std::string_view{"__pycache__"},  std::string_view{".pytest_cache"},
      std::string_view{".mypy_cache"},  std::string_view{".ruff_cache"},
      std::string_view{"target"},

      std::string_view{".gradle"},      std::string_view{".idea"},
      std::string_view{".vscode"},      std::string_view{".tox"},

      std::string_view{".DS_Store"},
  };

  const std::string name = dir.filename().string();
  for (std::string_view n : names) {
    if (name == n) {
      return true;
    }
  }
  return false;
}

bool hasCachedirTag(const fs::path &dir) {
  constexpr std::string_view MAGIC =
      "Signature: 8a477f597d28d172789f06886806bc55";

  const fs::path tag_path = dir / "CACHEDIR.TAG";
  int fd = open(tag_path.c_str(), O_RDONLY);
  if (fd == -1) {
    return false;
  }

  char buf[64] = {};
  ssize_t n = read(fd, buf, MAGIC.size());
  close(fd);

  if (n < static_cast<ssize_t>(MAGIC.size())) {
    return false;
  }
  return std::string_view(buf, MAGIC.size()) == MAGIC;
}

#ifdef __APPLE__
bool isMacosCachePath(const fs::path &dir) {
  static constexpr std::array tokens = {
      std::string_view{"/Library/Caches/"},
      std::string_view{"/Library/Containers/"},
      std::string_view{"/Library/Logs/"},
      std::string_view{"/Library/Saved Application State/"},
      std::string_view{"/Library/WebKit/"},
      std::string_view{"/Library/HTTPStorages/"},
      std::string_view{"/Library/Cookies/"},
      std::string_view{"/Library/Group Containers/"},
  };

  const std::string s = dir.string();
  for (std::string_view t : tokens) {
    if (s.find(t) != std::string::npos) {
      return true;
    }
  }
  return false;
}
#else
bool isMacosCachePath(const fs::path &) { return false; }
#endif

} // namespace

bool shouldSkip(const fs::path &dir) {
  return hasCacheLikeName(dir) || hasCachedirTag(dir) || isMacosCachePath(dir);
}

} // namespace cache_detector
