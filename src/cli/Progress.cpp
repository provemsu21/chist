#include "Progress.hpp"

namespace progress {

void NullProgress::updateLine(std::string_view, std::string_view) {}
void NullProgress::bar(std::string_view, size_t, size_t) {}
void NullProgress::finish(std::string_view) {}

void TtyProgress::updateLine(std::string_view label, std::string_view body) {
  std::lock_guard<std::mutex> lock(mutex_);
  tty_line::updateLine(label, body);
}

void TtyProgress::bar(std::string_view label, size_t done, size_t total) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!bar_) {
    bar_.emplace(total);
  }
  bar_->updateBar(label, done);
}

void TtyProgress::finish(std::string_view final_msg) {
  std::lock_guard<std::mutex> lock(mutex_);
  tty_line::finish(final_msg);
  bar_.reset();
}

} // namespace progress
