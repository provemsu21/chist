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

struct HashAlgorithm {
  virtual ~HashAlgorithm() = default;
  virtual std::string compute(const fs::path &path) const = 0;
  virtual std::string computeHead(const fs::path &path, size_t bytes) const = 0;
};

struct MD5Algorithm : HashAlgorithm {
  std::string compute(const fs::path &path) const override;
  std::string computeHead(const fs::path &path, size_t bytes) const override;
};

struct SHA256Algorithm : HashAlgorithm {
  std::string compute(const fs::path &path) const;
  std::string computeHead(const fs::path &path, size_t bytes) const;
};

template <HashAlgorithm Algo>
std::string getHash(const fs::path &path, Algo algo) {
  return algo.compute(path);
}

template <HashAlgorithm Algo>
std::string getHeadHash(const fs::path &path, size_t bytes, Algo algo) {
  return algo.computeHead(path, bytes);
}

} // namespace hasher

#endif // CORE_HASHER_HPP_
