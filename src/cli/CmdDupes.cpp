#include "../ToHuman.hpp"
#include "../cleaner/Cleaner.hpp"
#include "../core/Deduplicator.hpp"
#include "CommandRegistry.hpp"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

Command makeDupesCmd() {
  return Command{
      .name = "dupes",
      .description = "Find duplicate files by size and hash",
      .handler = [](const CmdArgs &args) {
        auto table = deduplicator::findDuplicates(args.path, args.algo);

        int group = 1;

        std::cout << "Found duplicates: " << '\n';

        std::cout << "To finish press \"q\" " << '\n';

        std::vector<fs::path> to_clean;
        for (auto &[hash, vec] : table) {
          std::cout << "Group " << group++ << '\n';
          std::cout << "Hash " << hash << '\n';
          for (auto &path : vec) {
            std::cout << path << '\n';
          }
          std::string apply;
          if (!args.yes) {
            std::cout << "Delete duplicates? " << "("
                      << static_cast<int>(vec.size()) - 1 << ") [y/n]";

            std::getline(std::cin, apply);
            if (apply == "q" || apply == "quit") {
              break;
            }
            if (apply == "n" || apply == "no")
              continue;
          }

          if (args.yes || apply == "y" || apply == "yes") {
            for (size_t i = 1; i < vec.size(); ++i) {
              if (!fs::is_regular_file(vec[i])) {
                std::cerr << "Failed to delete: " << vec[i].string() << '\n';
                continue;
              }
              to_clean.push_back(vec[i]);
            }
          }
          ++group;
        }

        auto metrics = cleaner::clean(to_clean);

        std::cout << "Freed " << human::toHuman(metrics.freed_bytes) << " ("
                  << metrics.deleted_count << " files)" << '\n';
      }};
}
