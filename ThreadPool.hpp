#pragma once
#include <cassert>
#include <atomic>
#include <list>
#include <queue>
#include <memory>
#include <thread>
#include <vector>
#include <mutex>
#include <future>
#include <algorithm>
#include <functional>
#include <condition_variable>


class ThreadPool {
public:
    enum EConstants {
        TASK_QUEUE_DEPTH = 1024,
    };

    enum EWorkerType {
        CORE_WORKER,
        CACHE_WORKER,
    };

    explicit ThreadPool(int);
    ThreadPool(int, int);
    ~ThreadPool();

    void Shutdown(bool join);

    template<typename _Func, typename... _Args>
    auto Schedule(_Func&& f, _Args&&... args)
        -> std::future<typename std::result_of<_Func(_Args...)>::type>;

    template<typename _Func, typename... _Args>
    void Enqueue(_Func&& f, _Args&&... args);

    // no default constructor
    ThreadPool() = delete;
    // noncopyable
    ThreadPool(const ThreadPool&) = delete;
    void operator=(const ThreadPool&) = delete;

private:
    using Task = std::function<void()>;

    void RunWork(EWorkerType type);

    template<typename _Task>
    bool Grow(_Task firstTask);

    template<typename _Task>
    bool PushTask(_Task task);

    void Suicide();

    // size of core workers.
    const int coreSize_;
    // limit the maximum size of the workers;
    const int limitSize_;

    // quit signal
    std::atomic_bool quit_;

    // need to keep track of threads so we can join them
    std::mutex workerMu_;
    std::list<std::thread> workers_;

    // synchronization
    std::mutex mu_;
    std::condition_variable pushCv_;
    std::condition_variable popCv_;

    // the task queue
    std::queue<Task> tasks_;

    std::vector<std::thread> dying_;
};

// This constructor set limitPoolSize to zero,
// to create a static size threadpool.
inline ThreadPool::ThreadPool(int fixedPoolSize)
: ThreadPool(fixedPoolSize, 0) {

}

// This constructor just launches some amount of workers to init a pool.
inline ThreadPool::ThreadPool(int corePoolSize, int limitPoolSize)
: coreSize_(corePoolSize > 0 ? corePoolSize : 0)
, limitSize_(limitPoolSize > coreSize_ ? limitPoolSize : (coreSize_ > 0 ? coreSize_ : 1))
, quit_(false) {

}

// The destructor joins all threads
inline ThreadPool::~ThreadPool() {
    Shutdown(true);

    mu_.lock();
    auto dying = std::move(dying_);
    mu_.unlock();

    for (auto& d : dying) {
        if (d.joinable())
            d.join();
    }
}

// Shut pool down first before destructed.
inline void ThreadPool::Shutdown(bool join) {
    quit_ = true;
    pushCv_.notify_all();
    popCv_.notify_all();

    if (join) {
        workerMu_.lock();
        auto workers = std::move(workers_);
        workerMu_.unlock();

        for (auto& worker : workers)
            if (worker.joinable())
                worker.join();
    }
}

// Add new work item to the pool
template<typename _Func, typename... _Args>
auto ThreadPool::Schedule(_Func&& f, _Args&&... args)
-> std::future<typename std::result_of<_Func(_Args...)>::type> {
    // Do not allow scheduling after stopping the pool
    if (quit_)
        throw std::runtime_error("Schedule on stopped ThreadPool");

    using Result = typename std::result_of<_Func(_Args...)>::type;

    // package the function and arguments to a task object.
    auto task = std::make_shared<std::packaged_task<Result()>>(
        std::bind(std::forward<_Func>(f), std::forward<_Args>(args)...));
    std::future<Result> result = task->get_future();

    if (Grow(task))
        return result;

    if (!PushTask(task))
        throw std::runtime_error("Schedule on stopped ThreadPool");

    return result;
}

// Add new work item to the pool, without any result request.
template<typename _Func, typename... _Args>
void ThreadPool::Enqueue(_Func&& f, _Args&&... args) {
    if (quit_) return; // no throws.

    // enqueue task without the future of results.
    auto task = std::make_shared<std::packaged_task<void()>>(
        std::bind(std::forward<_Func>(f), std::forward<_Args>(args)...));

    if (Grow(task))
        return;

    PushTask(task);
}

inline void ThreadPool::RunWork(EWorkerType type) {
    auto idleInterval = std::chrono::minutes(1);

    for (;;) {
        std::unique_lock<std::mutex> lock(mu_);
        pushCv_.wait_for(lock, idleInterval, [this] {
            return !tasks_.empty() || quit_;
        });

        if (!tasks_.empty()) {
            auto task = std::move(tasks_.front());
            tasks_.pop();

            lock.unlock();
            popCv_.notify_one();

            task();
        }
        else if (quit_) {
            break;
        }
        else {
            if (!dying_.empty()) {
                auto dying = std::move(dying_);
                lock.unlock();

                for (auto& d : dying) {
                    if (d.joinable())
                        d.join();
                }
            }

            if (type == CACHE_WORKER)
                break;
        }
    }
}

template<typename _Task>
inline bool ThreadPool::Grow(_Task firstTask) {
    std::lock_guard<std::mutex> guard(workerMu_);
    if (quit_)
        return false;

    // Assign to a new core worker directly.
    // Add at least one core worker. In case of core size set to zero.
    if (workers_.empty() || workers_.size() < coreSize_) {
        workers_.emplace_back([this, firstTask]() {
            (*firstTask)();
            RunWork(CORE_WORKER);
        });
        return true;
    }

    if (workers_.size() < limitSize_) {
        mu_.lock();
        auto taskCount = tasks_.size();
        mu_.unlock();

        if (taskCount > limitSize_) {
            workers_.emplace_back([this]() {
                RunWork(CACHE_WORKER);
                if (!quit_)
                    Suicide();
            });
        }
    }

    return false;
}

template<typename _Task>
inline bool ThreadPool::PushTask(_Task task) {
    {
        std::unique_lock<std::mutex> lock(mu_);
        popCv_.wait(lock, [this]() { return quit_ ||
            tasks_.size() < TASK_QUEUE_DEPTH;
        });

        if (quit_)
            return false;

        // enqueue the task.
        tasks_.emplace([task]() { (*task)(); });
    }
    pushCv_.notify_one();
    return true;
}

inline void ThreadPool::Suicide() {
    const auto curTid = std::this_thread::get_id();

    std::unique_lock<std::mutex> lock(workerMu_);
    auto it = std::find_if(workers_.begin(), workers_.end(),
        [curTid](const std::thread& th) { return th.get_id() == curTid; });
    if (it == workers_.end())
        return;

    auto curThrd = std::move(*it);
    workers_.erase(it);
    lock.unlock();

    std::lock_guard<std::mutex> guard(mu_);
    dying_.emplace_back(std::move(curThrd));
}
