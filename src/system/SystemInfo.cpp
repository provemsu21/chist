#include "SystemInfo.hpp"
#include <ranges>

using namespace systeminfo;
namespace ranges = std::ranges;

#ifdef __APPLE__

#include <mach/mach.h>
#include <sys/sysctl.h>

RAMStat getRamStat() {
  mach_port_t host = mach_host_self();
  vm_statistics64_data_t vmstat;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;

  kern_return_t kr = host_statistics64(
      host, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmstat), &count);

  if (kr != KERN_SUCCESS) {
    return {};
  }

  vm_size_t page_size;
  host_page_size(host, &page_size);

  host_basic_info_data_t basic_info;
  mach_msg_type_number_t basic_count = HOST_BASIC_INFO_COUNT;
  host_info(host, HOST_BASIC_INFO, reinterpret_cast<host_info_t>(&basic_info),
            &basic_count);

  RAMStat rs;
  rs.free = vmstat.free_count * page_size;
  rs.inactive = vmstat.inactive_count * page_size;
  rs.total = basic_info.max_mem;
  rs.used =
      (vmstat.active_count + vmstat.wire_count + vmstat.speculative_count) *
      page_size;

  return rs;
}

std::vector<ProcStat> getTopProcs(size_t limit) {
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
  size_t size = 0;
  int status = sysctl(mib, 4, nullptr, &size, nullptr, 0);
  if (status != 0)
    return {};

  std::vector<kinfo_proc> procs(size / sizeof(kinfo_proc));
  status = sysctl(mib, 4, procs.data(), &size, nullptr, 0);
  if (status != 0)
    return {};

  std::vector<ProcStat> procs_vec;
  for (const kinfo_proc &p : procs) {
    ProcStat ps;
    ps.pid = p.kp_proc.p_pid;
    ps.name = p.kp_proc.p_comm;
    ps.mem = 0;
    procs_vec.push_back(ps);
  }

  ranges::sort(procs_vec, ranges::greater(), &ProcStat::mem);
  if (procs_vec.size() > limit) {
    procs_vec.resize(limit);
  }

  return procs_vec;
}

#else
RAMStat getRamStat() {}
std::vector<ProcStat> getTopProcs(size_t limit) {}
#endif
