#ifndef CORE_THREAD_POOL_HPP_
#define CORE_THREAD_POOL_HPP_

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace threadpool {

class ThreadPool {
public:
  explicit ThreadPool(size_t n = std::thread::hardware_concurrency());

  template <typename F, typename... Args>
  auto submit(F &&new_task,
              Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
    using R = std::invoke_result_t<F, Args...>;

    auto bound =
        std::bind(std::forward<F>(new_task), std::forward<Args>(args)...);
    auto packaged = std::make_shared<std::packaged_task<R()>>(std::move(bound));

    std::future<R> fut = packaged->get_future();
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      tasks_.emplace([packaged] { (*packaged)(); });
    }
    cv_.notify_one();
    return fut;
  }

  ~ThreadPool();

private:
  std::condition_variable cv_;
  bool stop_ = false;
  std::mutex queue_mutex_;
  std::queue<std::function<void()>> tasks_;
  std::vector<std::thread> workers_;
  void workerCycle();
};

template <typename Cont>
concept Container = requires(Cont c) {
  { c.size() } -> std::convertible_to<std::size_t>;
  { c.begin() } -> std::input_iterator;
  { c.end() } -> std::input_iterator;
};

template <Container Container, typename F>
void parallel_for_each(const Container &container, F func) {
  static ThreadPool pool(std::thread::hardware_concurrency() * 2);
  static const size_t chunk_size = std::thread::hardware_concurrency() * 2;

  std::vector<std::future<void>> fut;
  fut.reserve(chunk_size);

  for (const auto &el : container) {
    fut.push_back(pool.submit([&el, &func] { func(el); }));

    if (fut.size() >= chunk_size) {
      for (std::future<void> &f : fut) {
        f.get();
      }
      fut.clear();
    }
  }

  for (std::future<void> &f : fut) {
    f.get();
  }
}
} // namespace threadpool

#endif // CORE_THREAD_POOL_HPP_
