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

class ThreadPool;
 
// our worker thread objects
class Worker {
public:
    Worker(ThreadPool &s) : pool(s) { }
    void operator()();
private:
    ThreadPool &pool;
};

// the actual thread pool
class ThreadPool {
public:
    ThreadPool(size_t);
    template<class T, class F>
    std::future<T> enqueue(F f);
    ~ThreadPool();
private:
    friend class Worker;

    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
void Worker::operator()()
{
    while(true)
    {
        std::unique_lock<std::mutex> lock(pool.queue_mutex);
        while(!pool.stop && pool.tasks.empty())
            pool.condition.wait(lock);
        if(pool.stop && pool.tasks.empty())
            return;
        std::function<void()> task(pool.tasks.front());
        pool.tasks.pop();
        lock.unlock();
        task();
    }
}
 
// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.push_back(std::thread(Worker(*this)));
}

// add new work item to the pool
template<class T, class F>
std::future<T> ThreadPool::enqueue(F f)
{
    // don't allow enqueueing after stopping the pool
    if(stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

    auto task = std::make_shared< std::packaged_task<T()> >(f);
    std::future<T> res = task->get_future();    
    {
        std::unique_lock<std::mutex> lock(queue_mutex);    
        tasks.push([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}
 
// the destructor joins all threads
ThreadPool::~ThreadPool()
{
    stop = true;
    condition.notify_all();
    for(size_t i = 0;i<workers.size();++i)
        workers[i].join();
}

#endif
