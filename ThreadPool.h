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
class ThreadPool {
public:
    ThreadPool(size_t);

    template<class F, class... Args>
    decltype(auto) enqueue(F&& f, Args&&... args);

    ~ThreadPool();
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    void newWorker();

    std::condition_variable tasks_full_flag;    //block enqueue if tasks queue is full
    // synchronization (task queue <-> workers)
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads=std::thread::hardware_concurrency())
{
    workers.reserve(threads);
    for (size_t i = 0; i < threads; ++i)
        newWorker();
}

// add new work item to the pool
template<class F, class... Args>
decltype(auto) ThreadPool::enqueue(F&& f, Args&&... args)
{
    using return_type = std::invoke_result_t<F,Args...>;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        if (tasks.size() >= maxQueueSize)
            tasks_full_flag.wait(lock, [&]() {return tasks.size() < maxQueueSize; });

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::lock_guard lock{ queue_mutex };
        stop = true;
    }
    condition.notify_all();
    for (auto& worker : workers)
        worker.join();
}

inline void ThreadPool::newWorker()
{
    auto& flag = tasks_full_flag;
    workers.emplace_back(
        [this, &flag]
        {
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [&] { return this->stop || !this->tasks.empty(); });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                    flag.notify_one();
                }
                task();
            }
        }
        );
}

#endif
