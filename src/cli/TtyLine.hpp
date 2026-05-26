#ifndef CLI_TTYLINE_HPP_
#define CLI_TTYLINE_HPP_

#include <array>
#include <string>
#include <string_view>

namespace tty_line {

void updateLine(std::string_view label, std::string_view body);
void finish(std::string_view final_msg = {});
void show_cursor();
void hide_cursor();

class Cursor {
public:
  Cursor();
  ~Cursor();
  Cursor(const Cursor &) = delete;
  Cursor &operator=(const Cursor &) = delete;
};

class ProgressBar {
public:
  explicit ProgressBar(size_t total);
  void updateBar(std::string_view label, size_t done);
  void finishProgress(std::string_view final_msg);

private:
  float calcProcent();
  std::string getBar(float procent);
  const std::array<char, 4> spinner_ = {'|', '/', '-', '\\'};
  const size_t total_;
  size_t done_;
  size_t spinner_state = 0;
};

} // namespace tty_line

#endif // CLI_TTYLINE_HPP_
