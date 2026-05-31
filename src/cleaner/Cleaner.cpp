#include "Cleaner.hpp"
#include <filesystem>
#include <system_error>

namespace cleaner {

CleanResult clean(const std::vector<fs::path> &files) {
  size_t deleted_cnt = 0;
  uintmax_t deleted_sz = 0;

  for (const auto &f : files) {
    std::error_code ec;
    uintmax_t fsize = 0;
    if (fs::is_directory(f, ec) && !ec) {
      for (const auto &entry : fs::recursive_directory_iterator(
               f, fs::directory_options::skip_permission_denied, ec)) {
        if (ec)
          break;
        std::error_code entry_ec;
        if (fs::is_regular_file(entry, ec) && !ec) {
          fsize += entry.file_size(entry_ec);
        }
      }

      auto removed_cnt = fs::remove_all(f, ec);
      if (!ec && removed_cnt > 0) {
        deleted_sz += fsize;
        ++deleted_cnt;
      }
    } else {
      fsize = fs::file_size(f, ec);
      if (ec)
        continue;
      if (fs::remove(f, ec) && !ec) {
        deleted_sz += fsize;
        ++deleted_cnt;
      }
    }
  }

  return {deleted_cnt, deleted_sz};
}
} // namespace cleaner
