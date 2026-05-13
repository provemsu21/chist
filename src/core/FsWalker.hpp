#ifndef CORE_FSWALKER_HPP_
#define CORE_FSWALKER_HPP_

#include <filesystem>
#include <functional>
#include <sys/stat.h>

namespace fswalker {

namespace fs = std::filesystem;

using Visitor = std::function<void(const fs::path &p, const struct stat &st)>;

void walk(const fs::path &p, const Visitor &visitor);

} // namespace fswalker

#endif // CORE_FSWALKER_HPP_
