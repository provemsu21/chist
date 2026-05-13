#include "../ToHuman.hpp"
#include "../system/SystemInfo.hpp"
#include "CommandRegistry.hpp"
#include <iomanip>
#include <iostream>
#include <string>

Command makeSysinfCmd() {
  return Command{
      .name = "sysinf",
      .description = "Show RAM statistics and top processes by memory",
      .handler = [](const CmdArgs &args) {
        auto ram_info = systeminfo::getRamStat();

        std::cout << "System information" << '\n';
        std::string line(30, '-');
        std::cout << line << '\n';

        std::cout << "RAM:" << '\n';
        std::cout << "  " << std::setw(10) << std::left
                  << "Total:" << human::toHuman(ram_info.total) << '\n';
        std::cout << "  " << std::setw(10) << std::left
                  << "Used:" << human::toHuman(ram_info.used) << '\n';
        std::cout << "  " << std::setw(10) << std::left
                  << "Free:" << human::toHuman(ram_info.free) << '\n';
        std::cout << "  " << std::setw(10) << std::left
                  << "Inactive:" << human::toHuman(ram_info.inactive) << '\n';
        std::cout << line << '\n';

        auto sys_info = systeminfo::getTopProcs(args.n);
        std::cout << "Top " << args.n << " processes by memory:" << '\n';
        std::cout << "  " << std::setw(8) << std::left << "PID" << std::setw(20)
                  << std::left << "NAME" << std::setw(10) << std::left
                  << "MEMORY" << '\n';

        for (auto &si : sys_info) {
          std::cout << "  " << std::setw(8) << std::left << si.pid
                    << std::setw(20) << std::left << si.name << std::setw(10)
                    << std::left << human::toHuman(si.mem) << '\n';
        }
      }};
}
