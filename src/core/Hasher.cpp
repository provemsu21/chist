#include "Hasher.hpp"
#include <CommonCrypto/CommonDigest.h>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>

using namespace hasher;

#ifdef __APPLE__

std::string MD5Algorithm::compute(const fs::path &path) const {
  CC_MD5_CTX c;
  CC_MD5_Init(&c);

  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return {};
  }

  char buf[4096];

  while (ssize_t n = read(fd, buf, sizeof(buf))) {
    if (n == -1) {
      close(fd);
      return {};
    }

    CC_MD5_Update(&c, buf, n);
  }
  close(fd);

  unsigned char hash[CC_MD5_DIGEST_LENGTH];
  CC_MD5_Final(hash, &c);

  char hex[CC_MD5_DIGEST_LENGTH * 2 + 1];

  for (int i = 0; i < CC_MD5_DIGEST_LENGTH; ++i) {
    snprintf(hex + i * 2, 3, "%02x", hash[i]);
  }

  return std::string(hex);
}

#else

std::string MD5Algorithm::compute(const fs::path &path) const {}

#endif
