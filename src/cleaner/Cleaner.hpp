#ifndef CLEANER_CLEANER_HPP_
#define CLEANER_CLEANER_HPP_

#include <cstdint>
#include <filesystem>
#include <vector>

namespace cleaner {

namespace fs = std::filesystem;

struct CleanResult {
  size_t deleted_count;
  uintmax_t freed_bytes;
};

CleanResult clean(const std::vector<fs::path> &files);
} // namespace cleaner

#endif // CLEANER_CLEANER_HPP_
