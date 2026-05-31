#ifndef CORE_HASHER_HPP_
#define CORE_HASHER_HPP_

#include <concepts>
#include <filesystem>
#include <string>

namespace hasher {

namespace fs = std::filesystem;

enum class HashType { MD5, SHA256 };

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

std::string getHash(const fs::path &path, HashType type);

std::string getHeadHash(const fs::path &path, size_t bytes, HashType type);

std::string hashBytes(const void *data, size_t len, HashType type);

} // namespace hasher

#endif // CORE_HASHER_HPP_
