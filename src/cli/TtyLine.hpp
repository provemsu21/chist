#ifndef CLI_TTYLINE_HPP_
#define CLI_TTYLINE_HPP_

#include <string_view>

namespace tty_line {

void updateLine(std::string_view label, std::string_view body);
void finish(std::string_view final_msg = {});

class Cursor {
public:
  Cursor();
  ~Cursor();
  Cursor(const Cursor &) = delete;
  Cursor &operator=(const Cursor &) = delete;
};
} // namespace tty_line

#endif // CLI_TTYLINE_HPP_
