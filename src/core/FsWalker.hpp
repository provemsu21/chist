#ifndef CORE_FSWALKER_HPP_
#define CORE_FSWALKER_HPP_

#include <filesystem>
#include <functional>
#include <sys/stat.h>

namespace fswalker {

namespace fs = std::filesystem;

struct Visitor {
  std::function<void(const fs::path &p, const struct stat &st)> onFile;
  std::function<void(const fs::path &p, const struct stat &st)> onDir;
  std::function<void(const fs::path &p)> onDirExit;
};

void walk(const fs::path &p, const Visitor &visitor);

} // namespace fswalker

#endif // CORE_FSWALKER_HPP_
