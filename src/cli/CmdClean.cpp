#include "../ToHuman.hpp"
#include "../cleaner/Cleaner.hpp"
#include "CommandRegistry.hpp"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

Command makeCleanCmd() {
  return Command{
      .name = "clean",
      .description = "Delete specified files with optional confirmation",
      .handler = [](const CmdArgs &args) {
        std::vector<fs::path> files;
        fs::path root = args.path;
        for (const fs::directory_entry &dir_entry :
             fs::recursive_directory_iterator(
                 root, fs::directory_options::skip_permission_denied)) {
          if (!dir_entry.is_regular_file())
            continue;
          std::string apply;
          if (!args.yes) {
            std::cout << "Delete " << dir_entry.path().string() << " ("
                      << human::toHuman(dir_entry.file_size()) << ") "
                      << "? [y/n]: ";
            std::getline(std::cin, apply);
            if (apply == "n" || apply == "no")
              continue;
          }
          if (args.yes || apply == "y" || apply == "yes")
            files.push_back(dir_entry.path());
        }
        auto metrics = cleaner::clean(files);

        std::cout << "Freed " << human::toHuman(metrics.freed_bytes) << " ("
                  << metrics.deleted_count << " files)" << '\n';
      }};
}
