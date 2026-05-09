#ifndef CORE_HASHER_HPP_
#define CORE_HASHER_HPP_

#include <concepts>
#include <filesystem>
#include <string>

namespace hasher {

namespace fs = std::filesystem;

template <typename Algo>
concept HashAlgorithm = requires(Algo a, const fs::path &path) {
  { a.compute(path) } -> std::convertible_to<std::string>;
};

struct MD5Algorithm {
  std::string compute(const fs::path &path) const;
};

struct SHA256Algorithm {
  std::string compute(const fs::path &path) const;
};

template <HashAlgorithm Algo>
std::string getHash(const fs::path &path, Algo algo) {
  return algo.compute(path);
}

} // namespace hasher

#endif // CORE_HASHER_HPP_
