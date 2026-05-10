#include "Analyzer.hpp"
#include <ranges>

using namespace analyzer;
namespace ranges = std::ranges;

DiskInfo getDiskInfo(const fs::path &path) {
  fs::space_info si = fs::space(path);
  DiskInfo di;
  di.available = si.available;
  di.capacity = si.capacity;
  di.free = si.free;

  return di;
}

std::vector<FileEntry> getTopFiles(const fs::path &path, size_t limit) {
  std::vector<FileEntry> files;

  for (const fs::directory_entry &dir_entry :
       fs::recursive_directory_iterator(path)) {
    if (!dir_entry.is_regular_file())
      continue;
    fs::path filepath = dir_entry.path();
    uintmax_t filesize = fs::file_size(filepath);
    files.push_back({filepath, filesize});
  }

  ranges::sort(files, ranges::greater(), &FileEntry::size);

  if (files.size() > limit)
    files.resize(limit);

  return files;
}
