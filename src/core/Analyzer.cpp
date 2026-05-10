#include "Analyzer.hpp"

#include <ranges>

using namespace analyzer;
namespace ranges = std::ranges;

namespace {
std::string toHuman(uintmax_t size) {
  std::vector<std::string> pref = {"b", "Kb", "Mb", "Gb", "Tb"};
  int pref_idx = 0;

  double tmp = size;
  while (tmp >= 1024 && pref_idx < (int)pref.size() - 1) {
    tmp /= 1024;
    pref_idx++;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f%s", tmp, pref[pref_idx].c_str());

  return std::string(buf);
}
} // namespace

DiskInfo getDiskInfo(const fs::path &path) {
  fs::space_info si = fs::space(path);
  DiskInfo di;
  di.available = toHuman(si.available);
  di.capacity = toHuman(si.capacity);
  di.free = toHuman(si.free);

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
