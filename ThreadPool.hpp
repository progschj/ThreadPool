#pragma once
#include <cassert>
#include <atomic>
#include <vector>
#include <list>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <algorithm>
#include <functional>
#include <condition_variable>


class ThreadPool {
public:
    explicit ThreadPool(size_t);
    ThreadPool(size_t, size_t);
    ~ThreadPool();

    template<class F, class... Args>
    auto Schedule(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    template<class F, class... Args>
    void Enqueue(F&& f, Args&&... args);

private:
    // no default constructor
    ThreadPool() = delete;
    // noncopyable
    ThreadPool(const ThreadPool&) = delete;
    void operator=(const ThreadPool&) = delete;

    void Work();
    void Assist();

    void SelfRemoveAssist();

    const size_t initial_;
    const size_t limits_;

    // synchronization
    std::mutex mutex_;
    int idles_;
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers_;

    std::list< std::thread > assists_;
    // the task queue
    std::queue< std::function<void()> > tasks_;

    std::condition_variable cond_;
    bool quit_;
};

// the constructor set threadsMax to zero,
// to create a static size threadpool.
inline ThreadPool::ThreadPool(size_t threads)
: ThreadPool(threads, 0) {}

// the constructor just launches some amount of workers to init a pool.
inline ThreadPool::ThreadPool(size_t threadsInit, size_t threadsMax)
: initial_(threadsInit > 0 ? threadsInit : 1)
, limits_(threadsMax > initial_ ? threadsMax : 0)
, idles_(0)
, quit_(false) {
    // init the fixed number of threads.
    for (size_t i = 0; i < initial_; ++i)
        workers_.emplace_back(&ThreadPool::Work, this);
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool() {
    mutex_.lock();
    quit_ = true;
    auto assists = std::move(assists_);
    mutex_.unlock();

    cond_.notify_all();

    for (auto& worker : workers_)
        if (worker.joinable())
            worker.join();

    for (auto& assist : assists)
        if (assist.joinable())
            assist.join();
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::Schedule(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
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
        assert(idles_ >= 0);
        if (idles_ == 0 && limits_ > initial_ && assists_.size() < limits_ - initial_)
            assists_.emplace_back(&ThreadPool::Assist, this);
    }
    cond_.notify_one();
    return res;
}

template<class F, class... Args>
void ThreadPool::Enqueue(F&& f, Args&&... args) {
    // enqueue task without the future of results.
    auto task = std::make_shared< std::packaged_task<void()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (quit_)
            return; // no throws.
        tasks_.emplace([task]() { (*task)(); });
        assert(idles_ >= 0);
        if (idles_ == 0 && limits_ > initial_ && assists_.size() < limits_ - initial_)
            assists_.emplace_back(&ThreadPool::Assist, this);
    }
    cond_.notify_one();
}

inline void ThreadPool::Work() {
    for (;;) {
        std::unique_lock<std::mutex> lock(mutex_);
        ++idles_;

        cond_.wait(lock, [this] {
            return quit_ || !tasks_.empty();
        });

        --idles_;

        if (tasks_.empty()) {
            if (quit_)
                return;
            continue;
        }

        auto task = std::move(tasks_.front());
        tasks_.pop();
        lock.unlock();

        task();
    }
}

inline void ThreadPool::Assist() {
    for (;;) {
        std::unique_lock<std::mutex> lock(mutex_);
        // increase the idle count
        ++idles_;

        // the expanded threads, wait for 1 minute idle and quit.
        cond_.wait_for(lock,std::chrono::minutes(1), [this] {
            return quit_ || !tasks_.empty();
        });
        --idles_;

        if (tasks_.empty()) {
            if (!quit_)
                SelfRemoveAssist();
            return;
        }

        auto task = std::move(tasks_.front());
        tasks_.pop();
        lock.unlock();

        task();
    }
}

inline void ThreadPool::SelfRemoveAssist() {
    auto it = std::find_if(assists_.begin(), assists_.end(), [](const std::thread& th) {
        return th.get_id() == std::this_thread::get_id();
    });
    if (it != assists_.end()) {
        it->detach();
        assists_.erase(it);
    }
}
