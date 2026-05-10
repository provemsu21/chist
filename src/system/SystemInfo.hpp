#ifndef SYSTEM_SYSTEMINFO_HPP_
#define SYSTEM_SYSTEMINFO_HPP_

#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>

namespace systeminfo {

struct RAMStat {
  uintmax_t total;
  uintmax_t used;
  uintmax_t free;
  uintmax_t inactive;
};

struct ProcStat {
  pid_t pid;
  std::string name;
  uintmax_t mem;
};

RAMStat getRamStat();
std::vector<ProcStat> getTopProcs(size_t limit = 10);

} // namespace systeminfo

#endif // SYSTEM_SYSTEMINFO_HPP_
