#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool;
 
// our worker thread objects
class Worker {
public:
    Worker(ThreadPool &s) : pool(s) { }
    void operator()();
private:
    ThreadPool &pool;
};

template<class T>
class Result {
    struct ResultImpl {
        ResultImpl() : value(T()), available(false) { }
        T value;
        bool available;
        std::mutex lock;
        std::condition_variable cond;
    };
public:
    Result() : impl(new ResultImpl()) { }
    bool available() const
    {
        std::unique_lock<std::mutex> ul(impl->lock);
        return impl->available;
    }
    void wait()
    {
        if(!impl)
            return;
        std::unique_lock<std::mutex> ul(impl->lock);
        if(impl->available)
            return;
        impl->cond.wait(ul);
    }
    void signal() const
    {
        std::unique_lock<std::mutex> ul(impl->lock);
        impl->available = true; impl->cond.notify_all();
    }
    bool valid() const
    { 
        std::unique_lock<std::mutex> ul(impl->lock);
        return static_cast<bool>(impl);
    }

    T& get()
    { 
        wait(); 
        return impl->value;
    }
    void set(T v) const
    {
        std::unique_lock<std::mutex> ul(impl->lock); 
        impl->value = v; 
    }
  
private:
    std::shared_ptr<ResultImpl> impl;
};

template<>
class Result<void> {
    struct ResultImpl {
        ResultImpl() : available(false) {  }
        bool available;
        std::mutex lock;
        std::condition_variable cond;
    };
public:
    Result() : impl(new ResultImpl()) { }

    bool available() const
    {
        std::unique_lock<std::mutex> ul(impl->lock);
        return impl->available;
    }
    void wait()
    {
        if(!impl)
            return;
        std::unique_lock<std::mutex> ul(impl->lock);
        if(impl->available)
            return;
        impl->cond.wait(ul);
    }
    void signal() const
    {
        std::unique_lock<std::mutex> ul(impl->lock);
        impl->available = true; impl->cond.notify_all();
    }
    bool valid() const
    { 
        std::unique_lock<std::mutex> ul(impl->lock);
        return static_cast<bool>(impl);
    }
private:
    std::shared_ptr<ResultImpl> impl;
};

// the actual thread pool
class ThreadPool {
public:
    ThreadPool(size_t);
    template<class T, class F>
    Result<T> enqueue(F f);
    ~ThreadPool();
private:
    friend class Worker;

    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::deque< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
void Worker::operator()()
{
    std::function<void()> task;
    while(true)
    {
        {
            std::unique_lock<std::mutex> lock(pool.queue_mutex);
            while(!pool.stop && pool.tasks.empty())
                pool.condition.wait(lock);
            if(pool.stop)
                return;
            task = pool.tasks.front();
            pool.tasks.pop_front();
        }
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

template<class T, class F>
struct CallAndSet {
    void operator()(const Result<T> &res, const F f)
    {
        res.set(f());
        res.signal();
    }
};

template<class F>
struct CallAndSet<void,F> {
    void operator()(const Result<void> &res, const F &f)
    {
        f();
        res.signal();
    }
};
 
// add new work item to the pool
template<class T, class F>
Result<T> ThreadPool::enqueue(F f)
{
    Result<T> res;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push_back(std::function<void()>(
        [f,res]()
        {
            CallAndSet<T,F>()(res, f);
        }));
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
