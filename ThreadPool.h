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

class ThreadPool
{
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool(void);
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void(void)> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(const size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        this->workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    while(!this->stop && this->tasks.empty())
                        this->condition.wait(lock);
                    if(this->stop && this->tasks.empty())
                        return;
                    std::function<void(void)> task(this->tasks.front());
                    this->tasks.pop();
                    lock.unlock();
                    task();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    // don't allow enqueueing after stopping the pool
    if(this->stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

    auto task = std::make_shared< std::packaged_task<return_type(void)> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        this->tasks.emplace([task](void){ (*task)(); });
    }
    this->condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool(void)
{
    {
        std::unique_lock<std::mutex> lock(this->queue_mutex);
        this->stop = true;
    }
    condition.notify_all();
    for(size_t i = 0;i<this->workers.size();++i)
        this->workers[i].join();
}

#endif
