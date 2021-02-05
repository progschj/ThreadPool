#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
constexpr size_t maxQueueSize = 100;

class ThreadPool
{
public:
    ThreadPool(size_t threads = std::thread::hardware_concurrency() - 1)
    {
        workers.reserve(threads);
        for (size_t i{}; i < threads; ++i)
            makeThread();
    }

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&... args)
    {
        using return_type = std::invoke_result_t<F, Args...>;
                                                                     //...v This need C++20!
        std::packaged_task<return_type()> task{[f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable { return f(std::forward<Args>(args)...); }};
        auto result = task.get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            if (tasks.size() >= maxQueueSize)
                queueStatus.wait(lock, [&]() { return tasks.size() < maxQueueSize; });

            tasks.emplace(std::move(task));//No need for std::shared_ptr anymore!
        }
        hasNewTask.notify_one();
        return result;
    }

    ~ThreadPool()
    {
        stop = true;
        hasNewTask.notify_all();
        for (auto &worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable queueStatus;
    std::condition_variable hasNewTask;
    std::atomic_bool stop{false};

    void makeThread()
    {
        workers.emplace_back(
            [this] {
                while (true)
                {
                    std::packaged_task<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->hasNewTask.wait(lock, [&] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        queueStatus.notify_one();
                    }
                    task();
                }
            });
    }
};

#endif
