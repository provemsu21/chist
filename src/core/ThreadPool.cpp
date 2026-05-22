#include "ThreadPool.hpp"
#include <memory>
#include <utility>

namespace threadpool {
void ThreadPool::workerCycle() {
  while (true) {
    std::function<void()> task;

    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      cv_.wait(lock, [this] { return !tasks_.empty() || stop_; });

      if (stop_ && tasks_.empty())
        return;

      task = std::move(tasks_.front());
      tasks_.pop();
    }

    task();
  }
}

ThreadPool::ThreadPool(size_t n) {
  for (size_t i = 0; i < n; ++i) {
    workers_.emplace_back([this] { workerCycle(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }
  cv_.notify_all();

  for (std::thread &t : workers_) {
    t.join();
  }
}

} // namespace threadpool
