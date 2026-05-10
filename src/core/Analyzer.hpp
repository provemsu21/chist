#ifndef CORE_ANALYZER_HPP_
#define CORE_ANALYZER_HPP_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace analyzer {
namespace fs = std::filesystem;
struct FileEntry {
  fs::path path;
  uintmax_t size;
};

struct DiskInfo {
  uintmax_t capacity;
  uintmax_t free;
  uintmax_t available;
};

DiskInfo getDiskInfo(const fs::path &path = "/");
std::vector<FileEntry> getTopFiles(const fs::path &path, size_t limit = 10);

} // namespace analyzer

#endif // CORE_ANALYZER_HPP_
