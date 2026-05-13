#include "CommandRegistry.hpp"
#include <iostream>

Command makeJunkCmd() {
  return Command{.name = "junk",
                 .description =
                     "Find and remove system junk: caches, logs and temp files",
                 .handler = [](const CmdArgs &args) {
                   std::cout << "Not implemented yet\n";
                 }};
}
