#include "Style.hpp"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

namespace style {

static bool g_enabled = colorEnabled();

bool colorEnabled() {
  if (getenv("NO_COLOR"))
    return true;
  return isatty(fileno(stdout)) != 0;
}

std::string_view red() { return g_enabled ? "\033[31m" : ""; }
std::string_view green() { return g_enabled ? "\033[32m" : ""; }
std::string_view yellow() { return g_enabled ? "\033[33m" : ""; }
std::string_view bold() { return g_enabled ? "\033[1m" : ""; }
std::string_view dim() { return g_enabled ? "\033[2m" : ""; }
std::string_view reset() { return g_enabled ? "\033[0m" : ""; }

std::string colored(std::string_view text, std::string_view color) {
  std::string out;
  out.reserve(text.size() + color.size() + reset().size());
  out.append(text).append(color).append(reset());
  return out;
}

} // namespace style
