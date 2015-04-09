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

/**
 * This is a function wrapper,that can wrap all different type packaged_task.
 * The code copy from of "C++ Concurrency IN ACTION".
 */
class function_wrapper
{
    struct impl_base
    {
        virtual void call()=0;
        virtual ~impl_base() {}
    };
    std::unique_ptr<impl_base> impl;
    template <typename F> struct impl_type: impl_base
    {
        F f;
        impl_type(F&& f_): f(std::move(f_)) {}
        void call()
        {
            f();
        }
    };
public:
    template<typename F>function_wrapper(F&& f):impl(new impl_type<F>(std::move(f))) {}
    void operator()()
    {
        impl->call();
    }
    function_wrapper()=default;
    function_wrapper(function_wrapper&& other):impl(std::move(other.impl)) {}
    function_wrapper& operator=(function_wrapper&& other)
    {
        impl=std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&)=delete;
    function_wrapper(function_wrapper&)=delete;
    function_wrapper& operator=(const function_wrapper&)=delete;

};

class ThreadPool
{
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task ptr queue
    std::queue< std::shared_ptr<function_wrapper> > tasks;

    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0; i<threads; ++i)
        workers.emplace_back(
            [this]
    {
        for(;;)
        {
            std::shared_ptr<function_wrapper> task;

            {
                std::unique_lock<std::mutex> lock(this->queue_mutex);
                this->condition.wait(lock,
                [this] { return this->stop || !this->tasks.empty(); });
                if(this->stop && this->tasks.empty())
                    return;
                //move shared_prt never throw exception,avoid terminate process.
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }

            (*task)();
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

    auto task =  std::packaged_task<return_type()> (
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task.get_future();

    auto task_ptr= std::make_shared<function_wrapper> (std::move(task));

    {
        std::lock_guard<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.push(std::move(task_ptr));
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
for(std::thread &worker: workers)
        worker.join();
}

#endif
