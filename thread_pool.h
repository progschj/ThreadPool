#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class ThreadPool {
  public:
    using Index = unsigned int;

    ThreadPool(Index n_threads = std::thread::hardware_concurrency());
    ~ThreadPool() noexcept;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&&) = delete;

    template <class Fn, class... Args>
    std::future<typename std::result_of<Fn(Args...)>::type> enqueue(Fn&& f, Args&&... args);

  private:
    using Task = std::function<void()>;

    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

inline ThreadPool::ThreadPool(Index n_threads) : stop_(false) {
    if (n_threads == 0 || n_threads > std::thread::hardware_concurrency()) {
        throw std::runtime_error("error! invalid number of threads");
    }

    for (Index i = 0; i < n_threads; ++i)
        workers_.emplace_back([this] {
            while (true) {
                Task task;

                {
                    std::unique_lock<std::mutex> lock(queue_mutex_);
                    condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                    lock.unlock();
                }

                task();
            }
        });
}

template <class Fn, class... Args>
std::future<typename std::result_of<Fn(Args...)>::type> ThreadPool::enqueue(Fn&& func, Args&&... args) {
    using return_type = typename std::result_of<Fn(Args...)>::type;
    using packaged_task = std::packaged_task<return_type()>;

    auto task = std::make_shared<packaged_task>(std::bind(std::forward<Fn>(func), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        if (stop_) {
            throw std::runtime_error("error! enqueue on stopped ThreadPool");
        }

        tasks_.emplace([task]() { (*task)(); });
        lock.unlock();
    }
    condition_.notify_one();
    return res;
}

inline ThreadPool::~ThreadPool() noexcept {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
        worker.join();
    }
}

#endif /* THREAD_POOL_H_ */
