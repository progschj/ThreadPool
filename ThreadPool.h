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

class semaphore{
  private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

  public:
    //If the initial value of count is less than 0, then it's a fake semaphore.
    semaphore(int count_ = 0):count(count_){;}
    
    void notify(){
      if(count < 0)
        return;
      std::unique_lock<std::mutex> lck(mtx);
      ++count;
      cv.notify_one();
    }
    
    void wait(){
      if(count < 0)
        return;   
      std::unique_lock<std::mutex> lck(mtx);
      while(count == 0){
        cv.wait(lck);
      }
      count--;
    }
};

class ThreadPool {
public:
    //If the queue_size is less than 0, then there's no max limit on the queue size.
    ThreadPool(size_t, int queue_size = -1);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();

private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    // semaphore which can limit the size of the task queue.        
    semaphore sem;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads, int queue_size)
    :   stop(false), sem(queue_size)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            [this]
            {
                for(;;)
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    while(!this->stop && this->tasks.empty())
                        this->condition.wait(lock);
                    if(this->stop && this->tasks.empty())
                        return;
                    std::function<void()> task(this->tasks.front());
                    this->tasks.pop();
                    lock.unlock();
                    task();
                    sem.notify();
                }
            }
        );
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    //Block if reach the max limit of the queue size.
    sem.wait();

    typedef typename std::result_of<F(Args...)>::type return_type;
    
    // don't allow enqueueing after stopping the pool
    if(stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push([task](){ (*task)(); });
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(size_t i = 0;i<workers.size();++i)
        workers[i].join();
}

#endif
