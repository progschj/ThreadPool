#pragma once

#include <atomic>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <condition_variable>
#include "ScopeGuard.hpp"


class ThreadPool
{
public:
    explicit ThreadPool(size_t);
    ThreadPool(size_t, size_t);
    ~ThreadPool();

    template<class F, class... Args>
    auto Schedule(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;

    template<class F, class... Args>
    void Enqueue(F&& f, Args&&... args);

private:

    // no default constructor
    ThreadPool() = delete;
    // noncopyable
    ThreadPool(const ThreadPool&) = delete;
    void operator=(const ThreadPool&) = delete;

    void ThreadRun(bool expanded);

    const size_t threadsInit_;
    const size_t threadsMax_;

    // synchronization
    std::mutex mutex_;
    int threadsIdle_;
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers_;
    // the task queue
    std::queue< std::function<void()> > tasks_;

    std::condition_variable condition_;
    bool quit_;
};

// the constructor set threadsMax to zero,
// to create a static size threadpool.
inline ThreadPool::ThreadPool(size_t threads)
    : ThreadPool(threads, 0)
{

}

// the constructor just launches some amount of workers to init a pool.
inline ThreadPool::ThreadPool(size_t threadsInit, size_t threadsMax)
    : threadsInit_(threadsInit > 0 ? threadsInit : 1)
    , threadsMax_(threadsMax > threadsInit_ ? threadsMax : 0)
    , threadsIdle_(0)
    , quit_(false)
{
    // init the fixed number of threads.
    for (size_t i = 0; i < threadsInit_; ++i)
        workers_.emplace_back(&ThreadPool::ThreadRun, this, false);
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        quit_ = true;
    }
    condition_.notify_all();
    for (auto& worker : workers_)
        if (worker.joinable())
            worker.join();
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::Schedule(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    // package the function and arguments to a task object.
    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();

    {
        std::lock_guard<std::mutex> lock(mutex_);

        // don't allow scheduling after stopping the pool
        if (quit_)
            throw std::runtime_error("Schedule on stopped ThreadPool");

        // enqueue the task.
        tasks_.emplace([task]() { (*task)(); });

        // no idles, start a new one to expand the pool.
        assert(threadsIdle_ >= 0);
        if (threadsIdle_ == 0 && workers_.size() < threadsMax_)
            workers_.emplace_back(&ThreadPool::ThreadRun, this, true);
    }
    condition_.notify_one();
    return res;
}

template<class F, class... Args>
void ThreadPool::Enqueue(F&& f, Args&&... args)
{
    // enqueue task without the future of results.
    auto task = std::make_shared< std::packaged_task<void()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (quit_)
            return; // no throws.
        tasks_.emplace([task]() { (*task)(); });
        assert(threadsIdle_ >= 0);
        if (threadsIdle_ == 0 && workers_.size() < threadsMax_)
            workers_.emplace_back(&ThreadPool::ThreadRun, this, true);
    }
    condition_.notify_one();
}

inline void ThreadPool::ThreadRun(bool expanded)
{
    for (;;)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);

            // increase the idle count
            ++threadsIdle_;
            // and reset the count on exit this scope.
            ON_SCOPE_EXIT([this] { --threadsIdle_; });

            if (!expanded)  // the initialized threads, just wait forever.
                condition_.wait(lock,
                    [this] { return quit_ || !tasks_.empty(); });
            else if (!condition_.wait_for(lock, std::chrono::minutes(1),    // the expanded threads, wait for 1 minute idle and quit.
                    [this] { return quit_ || !tasks_.empty(); }))
            {
                // remove thread object of self on quit.
                auto it = std::find_if(workers_.begin(), workers_.end(),
                    [](const std::thread& th) { return th.get_id() ==
                    std::this_thread::get_id(); });
                if (it != workers_.end())
                {
                    it->detach();
                    workers_.erase(it);
                }
                return;
            }

            // check and get a task object.
            if (quit_ && tasks_.empty())
                return;
            task = std::move(tasks_.front());
            tasks_.pop();
        }

        if (task)
            task();
    }
}
