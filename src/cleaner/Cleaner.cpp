#include "Cleaner.hpp"
#include <filesystem>

using namespace cleaner;

CleanResult clean(const std::vector<fs::path> &files) {
  size_t deleted_cnt = 0;
  uintmax_t deleted_sz = 0;

  for (auto &f : files) {
    uintmax_t filesize = fs::file_size(f);
    bool removed = fs::remove(f);
    if (removed) {
      deleted_sz += filesize;
      ++deleted_cnt;
    }
  }

  return {deleted_cnt, deleted_sz};
}
