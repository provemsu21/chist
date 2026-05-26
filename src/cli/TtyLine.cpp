#include "Style.hpp"
#include "TtyLine.hpp"
#include <Log.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
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
clock::time_point g_last_render_line = clock::time_point{};
clock::time_point g_last_render_bar = clock::time_point{};

bool updatePerm(clock::time_point &last_render) {
  // return true;
  if (!g_is_tty)
    return false;
  auto now = clock::now();
  if (now - last_render >= kMinInterval) {
    last_render = now;
    return true;
  }
  return false;
}

} // namespace

void updateLine(std::string_view label, std::string_view body) {
  if (!updatePerm(g_last_render_line))
    return;

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

ProgressBar::ProgressBar(size_t total) : total_(total) {}

void ProgressBar::updateBar(std::string_view label, size_t done) {
  if (!updatePerm(g_last_render_bar))
    return;
  done_ = done;
  float procent = calcProcent();
  std::string bar = getBar(procent);

  std::string spinner = style::colored(
      std::string(1, spinner_[spinner_state % 4]), style::yellow());
  spinner_state++;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << procent << "% (" << std::setw(6)
      << done_ << "/" << std::setw(6) << total_ << ")";
  std::string stat = oss.str();

  std::string full_line = spinner + " " + label.data() + bar + " " + stat;
  std::cout << kClearLine << full_line << std::flush;
}

void ProgressBar::finishProgress(std::string_view final_msg) {
  finish(final_msg);
}

float ProgressBar::calcProcent() {
  return static_cast<float>(done_) / total_ * 100.0f;
}

std::string ProgressBar::getBar(float procent) {
  size_t progress = static_cast<size_t>(procent / 100.0f * 20);
  std::string bar = "[";
  for (size_t i = 0; i < 20; ++i) {
    if (i < progress) {
      bar += '#';
    } else {
      bar += '-';
    }
  }
  bar += ']';

  bar[21] = ']';
  return bar;
}

} // namespace tty_line
