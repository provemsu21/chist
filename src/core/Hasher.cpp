#include "Hasher.hpp"

#include <array>
#include <cerrno>
#include <cstdio>
#include <limits>

#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace hasher {

std::string getHash(const fs::path &path, HashType type) {
  if (type == HashType::MD5) {
    return MD5Algorithm{}.compute(path);
  }
  return SHA256Algorithm{}.compute(path);
}

std::string getHeadHash(const fs::path &path, size_t bytes, HashType type) {
  if (type == HashType::MD5) {
    return MD5Algorithm{}.computeHead(path, bytes);
  }
  return SHA256Algorithm{}.computeHead(path, bytes);
}

#ifdef __APPLE__

namespace {

template <typename Update>
bool read_chunks(const fs::path &path, Update update,
                 size_t max_bytes = std::numeric_limits<size_t>::max()) {
  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return false;
  }

  std::array<char, 64 * 1024> buf;
  size_t total = 0;
  while (total < max_bytes) {
    size_t want = std::min(max_bytes - total, buf.size());
    ssize_t n;
    do {
      n = read(fd, buf.data(), want);
    } while (n < 0 && errno == EINTR);

    if (n == 0)
      break;

    if (n < 0) {
      close(fd);
      return false;
    }

    update(buf.data(), static_cast<size_t>(n));
    total += static_cast<size_t>(n);
  }

  close(fd);
  return true;
}

template <size_t N> std::string toHex(const unsigned char (&raw)[N]) {
  char hex[N * 2 + 1];

  for (int i = 0; i < N; ++i) {
    snprintf(hex + i * 2, 3, "%02x", raw[i]);
  }

  return std::string(hex);
}
} // namespace

std::string MD5Algorithm::compute(const fs::path &path) const {
  CC_MD5_CTX c;
  CC_MD5_Init(&c);
  bool ok = read_chunks(path, [&c](const char *buf, size_t n) {
    CC_MD5_Update(&c, buf, static_cast<CC_LONG>(n));
  });
  if (!ok) {
    return {};
  }
  unsigned char hash[CC_MD5_DIGEST_LENGTH];

  if (CC_MD5_Final(hash, &c) != 1) {
    return {};
  }

  return toHex(hash);
}

std::string MD5Algorithm::computeHead(const fs::path &path,
                                      size_t bytes) const {
  CC_MD5_CTX c;
  CC_MD5_Init(&c);
  bool ok = read_chunks(
      path,
      [&c](const char *buf, size_t n) {
        CC_MD5_Update(&c, buf, static_cast<CC_LONG>(n));
      },
      bytes);
  if (!ok) {
    return {};
  }
  unsigned char hash[CC_MD5_DIGEST_LENGTH];

  if (CC_MD5_Final(hash, &c) != 1) {
    return {};
  }
  return toHex(hash);
}

std::string SHA256Algorithm::compute(const fs::path &path) const {
  CC_SHA256_CTX c;
  CC_SHA256_Init(&c);
  bool ok = read_chunks(path, [&c](const char *buf, size_t n) {
    CC_SHA256_Update(&c, buf, static_cast<CC_LONG>(n));
  });
  if (!ok) {
    return {};
  }
  unsigned char hash[CC_SHA256_DIGEST_LENGTH];

  if (CC_SHA256_Final(hash, &c) != 1) {
    return {};
  }

  return toHex(hash);
}

std::string SHA256Algorithm::computeHead(const fs::path &path,
                                         size_t bytes) const {
  CC_SHA256_CTX c;
  CC_SHA256_Init(&c);
  bool ok = read_chunks(
      path,
      [&c](const char *buf, size_t n) {
        CC_SHA256_Update(&c, buf, static_cast<CC_LONG>(n));
      },
      bytes);
  if (!ok) {
    return {};
  }
  unsigned char hash[CC_SHA256_DIGEST_LENGTH];
  if (CC_SHA256_Final(hash, &c) != 1) {
    return {};
  }
  return toHex(hash);
}

std::string hashBytes(const void *data, size_t len, HashType type) {
  if (type == HashType::MD5) {
    unsigned char digest[CC_MD5_DIGEST_LENGTH];
    CC_MD5(data, static_cast<CC_LONG>(len), digest);
    return toHex(digest);
  } else {
    unsigned char digest[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(data, static_cast<CC_LONG>(len), digest);
    return toHex(digest);
  }

#else

std::string MD5Algorithm::compute(const fs::path &path) const {}
std::string SHA256Algorithm::compute(const fs::path &path) const {}
std::string hashBytes(const void *data, size_t len, HashType type) {}
#endif
}
} // namespace hasher
