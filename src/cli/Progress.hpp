#ifndef CLI_PROGRESS_HPP_
#define CLI_PROGRESS_HPP_

#include "TtyLine.hpp"
#include <mutex>
#include <optional>
#include <string_view>

namespace progress {

class IProgress {
public:
  virtual ~IProgress() = default;

  virtual void updateLine(std::string_view label, std::string_view body) = 0;

  virtual void bar(std::string_view label, size_t done, size_t total) = 0;

  virtual void finish(std::string_view final_msg = {}) = 0;
};

class NullProgress : public IProgress {
public:
  void updateLine(std::string_view, std::string_view) override;
  void bar(std::string_view, size_t, size_t) override;
  void finish(std::string_view) override;
};

class TtyProgress : public IProgress {
public:
  void updateLine(std::string_view label, std::string_view body) override;
  void bar(std::string_view label, size_t done, size_t total) override;
  void finish(std::string_view final_msg) override;

private:
  std::mutex mutex_;
  std::optional<tty_line::ProgressBar> bar_;
};

} // namespace progress

#endif // CLI_PROGRESS_HPP_
