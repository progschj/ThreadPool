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

class ThreadPool {
public:
    ThreadPool();
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool() noexcept;
private:
    void work();
    void recycle();

    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
    int waiters;

    // for recycle idle threads
    static thread_local bool working;
    std::atomic< int > maxIdle;
    std::thread monitor;
};

thread_local bool ThreadPool::working = true;
 
// the constructor just launches monitor thread
inline ThreadPool::ThreadPool()
    :   stop(false), waiters(0)
{
    maxIdle = std::max< int >(1, static_cast<int>(std::thread::hardware_concurrency()));
    monitor = std::thread([this]() { this->recycle(); } );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });

        // if there is no idle thread, create one, do not let this task wait
        if (waiters == 0)
        {
            std::thread  t([this]() { this->work(); } );
            workers.push_back(std::move(t));
        }
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool() noexcept
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(auto& worker: workers)
        worker.join();

    monitor.join();
}

inline void ThreadPool::work() 
{
    working = true;

    while (working)
    {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);

            ++ waiters; // incr waiter count
            this->condition.wait(lock,
                [this]{ return this->stop || !this->tasks.empty(); });
            --  waiters; // decr waiter count

            if(this->stop && this->tasks.empty())
                return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
        }

        task();
    }

    // if reach here, this thread is recycled by monitor thread
}

inline void ThreadPool::recycle() 
{
    while (!stop)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        if (stop)
            return;
        
        auto nw = waiters;
        while (nw -- > maxIdle)
        {
            // the thread which fetch this task item will exit
            tasks.emplace([this]() { working = false; });
            condition.notify_one();
        }
    }
}

#endif
