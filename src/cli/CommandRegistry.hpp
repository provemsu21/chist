#ifndef CLI_COMMANDREGISTRY_HPP_
#define CLI_COMMANDREGISTRY_HPP_

#include "../core/Deduplicator.hpp"
#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

struct CmdArgs;

namespace fs = std::filesystem;
using CmdHandler = std::function<void(const CmdArgs &)>;

struct CmdArgs {
  std::string command;
  fs::path path = fs::current_path();
  bool yes = false;
  size_t n = 10;
  deduplicator::HashType algo = deduplicator::HashType::MD5;
};

struct Command {
  std::string name;
  std::string description;
  std::function<void(const CmdArgs &)> handler;
};

class CommandRegistry {
public:
  void registerCommand(Command cmd);
  void dispatch(int argc, char **argv);

private:
  void printHelp();
  CmdArgs parseArgs(int argc, char **argv);
  std::unordered_map<std::string, Command> commands_;
  std::vector<std::string> order_;
};

#endif // CLI_COMMANDREGISTRY_HPP_
