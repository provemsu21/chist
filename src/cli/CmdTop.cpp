#include "../ToHuman.hpp"
#include "../core/Analyzer.hpp"
#include "CommandRegistry.hpp"
#include <iostream>

Command makeTopCmd() {
  return Command{.name = "top",
                 .description = "Show top N largest files in directory",
                 .handler = [](const CmdArgs &args) {
                   auto files = analyzer::getTopFiles(args.path, args.n);

                   std::cout << "Top " << args.n << " largest files:" << '\n';

                   for (auto &f : files) {
                     std::cout << human::toHuman(f.size) << ": "
                               << f.path.string() << '\n';
                   }
                 }};
}
