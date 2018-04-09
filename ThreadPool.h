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
	
	private:
		//工作线程
		std::vector< std::thread > workers;
		//任务队列
		std::queue< std::function<void()> > tasks; 
	
		//线程同步
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	
	public:
		ThreadPool(size_t);
		~ThreadPool();

		//添加任务
		template<typename F, typename... Args>
		auto enqueue(F&& f, Args&&... args) 
			-> std::future<typename std::result_of<F(Args...)>::type>;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads):stop(false)
{
    for(size_t i = 0;i<threads;i++)
        workers.emplace_back(
            [=]()
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
						//抽取任务
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
					//执行任务
                    task();
                }
            }
        );
}

// add new work item to the pool
template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

	//封装任务为void() 类型 ，方便执行
    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
		
		//添加任务
        tasks.emplace([=](){ (*task)(); });
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
    for(auto &worker: workers)
        worker.join();
}

#endif
