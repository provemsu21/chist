#include "../ToHuman.hpp"
#include "../core/Analyzer.hpp"
#include "CommandRegistry.hpp"
#include <iomanip>
#include <iostream>
#include <string>

Command makeScanCmd() {
  return Command{
      .name = "scan",
      .description = "Analyze disk usage: show total, used and free space",
      .handler = [](const CmdArgs &args) {
        auto disk_info = analyzer::getDiskInfo(args.path);
        std::cout << "Disk usage for: " << args.path.string() << '\n';
        std::string line(30, '-');
        std::cout << line << '\n';
        std::cout << std::setw(10) << std::left
                  << "Total:" << human::toHuman(disk_info.capacity) << '\n';
        std::cout << std::setw(10) << std::left << "Used:"
                  << human::toHuman(disk_info.capacity - disk_info.free)
                  << '\n';
        std::cout << std::setw(10) << std::left
                  << "Free:" << human::toHuman(disk_info.free) << '\n';
        std::cout << std::setw(10) << std::left
                  << "Available:" << human::toHuman(disk_info.available)
                  << '\n';
      }};
}
