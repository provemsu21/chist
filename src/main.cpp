#include "Log.hpp"
#include "cli/CmdClean.hpp"
#include "cli/CmdDupes.hpp"
#include "cli/CmdJunk.hpp"
#include "cli/CmdScan.hpp"
#include "cli/CmdSysinf.hpp"
#include "cli/CmdTop.hpp"
#include "cli/CommandRegistry.hpp"
#include "cli/TtyLine.hpp"
#include <csignal>

void handleSigint(int) {
  LOG_DEBUG("SIGINT received");
  tty_line::show_cursor();
  std::_Exit(130);
}

int main(int argc, char **argv) {
  std::signal(SIGINT, handleSigint);
  CommandRegistry registry;
  registry.registerCommand(makeScanCmd());
  registry.registerCommand(makeDupesCmd());
  registry.registerCommand(makeTopCmd());
  registry.registerCommand(makeCleanCmd());
  registry.registerCommand(makeSysinfCmd());
  registry.registerCommand(makeJunkCmd());

  registry.dispatch(argc, argv);
  return 0;
}
