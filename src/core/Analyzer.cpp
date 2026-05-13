#include "Analyzer.hpp"
#include "FsWalker.hpp"
#include <ranges>
#include <system_error>

namespace analyzer {
namespace ranges = std::ranges;

DiskInfo getDiskInfo(const fs::path &path) {
  std::error_code ec;
  fs::space_info si = fs::space(path, ec);
  if (ec)
    return {};
  DiskInfo di;
  di.available = si.available;
  di.capacity = si.capacity;
  di.free = si.free;

  return di;
}

std::vector<FileEntry> getTopFiles(const fs::path &path, size_t limit) {
  std::vector<FileEntry> files;

  fswalker::walk(path, [&files](const fs::path &p, const struct stat &st) {
    files.push_back({p, static_cast<uintmax_t>(st.st_size)});
  });

  ranges::sort(files, ranges::greater(), &FileEntry::size);

  if (files.size() > limit)
    files.resize(limit);

  return files;
}
} // namespace analyzer
