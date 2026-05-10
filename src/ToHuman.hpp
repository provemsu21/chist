#ifndef TOHUMAN_HPP_
#define TOHUMAN_HPP_

#include <string>
#include <vector>

namespace human {
inline std::string toHuman(uintmax_t size) {
  std::vector<std::string> pref = {"B", "KB", "MB", "GB", "TB"};
  int pref_idx = 0;

  double tmp = size;
  while (tmp >= 1024 && pref_idx < (int)pref.size() - 1) {
    tmp /= 1024;
    pref_idx++;
  }

  char buf[32];
  snprintf(buf, sizeof(buf), "%.1f%s", tmp, pref[pref_idx].c_str());

  return std::string(buf);
}
} // namespace human

#endif // TOHUMAN_HPP_
