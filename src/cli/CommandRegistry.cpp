#include "../core/Hasher.hpp"
#include "CommandRegistry.hpp"
#include <iomanip>
#include <iostream>

void CommandRegistry::registerCommand(Command cmd) {
  order_.push_back(cmd.name);
  commands_[cmd.name] = std::move(cmd);
}

void CommandRegistry::dispatch(int argc, char **argv) {
  if (argc < 2) {
    printHelp();
    return;
  }

  CmdArgs args = parseArgs(argc, argv);
  auto it = commands_.find(args.command);
  if (it == commands_.end()) {
    std::cerr << "Unknown command: " << args.command << "\n";
    printHelp();
    return;
  }

  it->second.handler(args);
}

CmdArgs CommandRegistry::parseArgs(int argc, char **argv) {
  CmdArgs args;
  if (argc < 2)
    return args;

  args.command = argv[1];

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--yes" || arg == "-y") {
      args.yes = true;
    } else if (arg == "-n" && i + 1 < argc) {
      args.n = std::stoul(argv[++i]);
    } else if (arg == "--algo" && i + 1 < argc) {
      std::string algo = argv[++i];
      if (algo == "sha256")
        args.algo = hasher::HashType::SHA256;
    } else if (arg[0] != '-') {
      args.path = arg;
    }
  }

  return args;
}

void CommandRegistry::printHelp() {
  std::cout << "Usage: chist <command> [options] [path]\n";
  std::cout << "Commands:\n";
  for (const auto &name : order_) {
    const auto &cmd = commands_.at(name);
    std::cout << std::setw(10) << std::left << cmd.name << cmd.description
              << '\n';
  }

  std::cout << "Options:\n";
  std::cout << "  --yes, -y    Skip confirmation\n";
  std::cout << "  -n <num>     Number of results (default: 10)\n";
  std::cout << "  --algo       Hash algorithm: md5|sha256\n";
}
