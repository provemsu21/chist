#include "cli/CmdClean.hpp"
#include "cli/CmdDupes.hpp"
#include "cli/CmdJunk.hpp"
#include "cli/CmdScan.hpp"
#include "cli/CmdSysinf.hpp"
#include "cli/CmdTop.hpp"
#include "cli/CommandRegistry.hpp"

int main(int argc, char **argv) {
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
