#include "TtyLine.hpp"
#include <chrono>
#include <iostream>
#include <locale>
#include <sys/ioctl.h>
#include <unistd.h>

namespace tty_line {

namespace {
size_t getTtyWidth() {
  struct winsize ws;
  if (ioctl(fileno(stdout), TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
    return ws.ws_col;
  }
  return 80;
}

std::string truncate(std::string_view s, size_t max_width) {
  if (s.size() <= max_width)
    return std::string(s);
  if (max_width < 4)
    return std::string(s.substr(0, max_width));

  size_t tail = max_width - 3;
  return "..." + std::string(s.substr(s.size() - tail));
}

const bool g_is_tty = isatty(fileno(stdout)) != 0;
constexpr std::string_view kClearLine = "\r\033[K";

using clock = std::chrono::steady_clock;
constexpr auto kMinInterval = std::chrono::milliseconds(50);
clock::time_point g_last_render = clock::time_point{};

} // namespace

void updateLine(std::string_view label, std::string_view body) {
  if (!g_is_tty)
    return;
  auto now = clock::now();
  if (now - g_last_render < kMinInterval)
    return;
  g_last_render = now;

  size_t width = getTtyWidth();
  size_t budget = width > label.size() ? width - label.size() : 20;
  std::string shown = truncate(body, budget);

  std::cout << kClearLine << label << shown << std::flush;
}

void finish(std::string_view final_msg) {
  if (!g_is_tty)
    return;
  std::cout << kClearLine << final_msg << std::endl;
}

void show_cursor() { std::cout << "\033[?25h" << std::flush; }

void hide_cursor() { std::cout << "\033[?25l" << std::flush; }

Cursor::Cursor() {
  if (g_is_tty)
    hide_cursor();
}

Cursor::~Cursor() {
  if (g_is_tty)
    show_cursor();
}

} // namespace tty_line
