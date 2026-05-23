#ifndef CORE_CACHEDETECTOR_HPP_
#define CORE_CACHEDETECTOR_HPP_

#include <filesystem>

namespace cache_detector {
namespace fs = std::filesystem;

bool isCache(const fs::path &p);
} // namespace cache_detector

#endif // CORE_CACHEDETECTOR_HPP_
