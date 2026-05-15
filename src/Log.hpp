#ifndef LOG_HPP_
#define LOG_HPP_

#include <chrono>
#include <iostream>

#ifdef CHIST_DEBUG
#define LOG_DEBUG(expr)                                                        \
  do {                                                                         \
    std::cerr << "[debug] " << expr << '\n';                                   \
  } while (0)
#else
#define LOG_DEBUG(expr)                                                        \
  do {                                                                         \
  } while (0)
#endif

#define LOG_INFO(expr)                                                         \
  do {                                                                         \
    std::cerr << "[info] " << expr << '\n';                                    \
  } while (0)
#define LOG_ERROR(expr)                                                        \
  do {                                                                         \
    std::cerr << "[error] " << expr << '\n';                                   \
  } while (0)

class ScopedTimer {
public:
  explicit ScopedTimer(const char *name)
      : name_(name), start_(std::chrono::steady_clock::now()) {}
  ~ScopedTimer() {
    auto elapsed = std::chrono::steady_clock::now() - start_;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

    if (ms.count() >= 1000) {
      auto sec = std::chrono::duration<double>(elapsed);
      std::cerr << "[time] " << name_ << ": " << sec.count() << "s\n";
      return;
    }

    std::cerr << "[time] " << name_ << ": " << ms.count() << "ms\n";
  }

private:
  const char *name_;
  std::chrono::steady_clock::time_point start_;
};

#ifdef CHIST_DEBUG
#define TIME_SCOPE(name) ScopedTimer _timer_##__LINE__(name)
#else
#define TIME_SCOPE(name)
#endif

#endif // LOG_HPP_
