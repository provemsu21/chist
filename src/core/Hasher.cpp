#include "Hasher.hpp"
#include <CommonCrypto/CommonDigest.h>
#include <fcntl.h>
#include <unistd.h>

namespace hasher {

#ifdef __APPLE__

namespace {
template <typename Update>
void read_chunks(const fs::path &path, Update update) {
  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return;
  }

  char buf[4096];

  while (ssize_t n = read(fd, buf, sizeof(buf))) {
    if (n == -1) {
      close(fd);
      return;
    }
    update(buf, n);
  }
  close(fd);
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

  read_chunks(path,
              [&c](const char *buf, ssize_t n) { CC_MD5_Update(&c, buf, n); });

  unsigned char hash[CC_MD5_DIGEST_LENGTH];

  if (CC_MD5_Final(hash, &c) != 1) {
    return {};
  }

  return toHex(hash);
}

std::string SHA256Algorithm::compute(const fs::path &path) const {
  CC_SHA256_CTX c;
  CC_SHA256_Init(&c);

  read_chunks(
      path, [&c](const char *buf, ssize_t n) { CC_SHA256_Update(&c, buf, n); });

  unsigned char hash[CC_SHA256_DIGEST_LENGTH];

  if (CC_SHA256_Final(hash, &c) != 1) {
    return {};
  }

  return toHex(hash);
}

#else

std::string MD5Algorithm::compute(const fs::path &path) const {}
std::string SHA256Algorithm::compute(const fs::path &path) const {}

#endif
} // namespace hasher
