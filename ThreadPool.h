// Purpose: Thread pool

// Based on https://github.com/progschj/ThreadPool

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool final
{
  public:
	 ThreadPool (size_t threads);
	 ~ThreadPool ();

	// Enqueue task and return std::future<>
	 template<class F, class... Args>
	 auto enqueue (F&& f, Args&&... args) 
		-> std::future<typename std::result_of<F (Args...)>::type>;

	// Enqueue task with checked return value
	 template<class F, class... Args>
	 auto enqueueAndDetach (F&& f, Args&&... args) 
		-> void;

  private:
	 // Keep track of threads, so they can be joined
	 std::vector<std::thread> workers;
	 // Task queue
	 std::queue<std::function<void ()>> tasks;

	 // Synchronization
	 using Lock = std::unique_lock<std::mutex>;
	 std::mutex queue_mutex;
	 std::condition_variable condition;
	 bool stop = false;
};

// the constructor just launches some amount of workers
ThreadPool::ThreadPool (size_t threads)
{
  for (size_t i = 0; i < threads; ++i)
  {
	 // Worker execution loop
	 workers.emplace_back
	 (
		[this]
		{
		   for(;;)
		   {
			  // Task to execute
			  std::function<void ()> task;

			  // Wait for additional work signal
			  { // Critical section
				 // Wait to be notified of work
				 Lock lock (this->queue_mutex);
				 this->condition.wait
				 (
					lock,
					[this](){ return this->stop || !this->tasks.empty (); }
				 );

				 // Exit the thread if stopping and no work remains
				 if(this->stop && this->tasks.empty ())
					return;

				 // Dequeue the next task
				 task = std::move (this->tasks.front ());
				 this->tasks.pop ();
			  } // End critical section

			  // Execute
			  task ();
		   }
		}
	 );
  }
}

// Add a new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue (F&& f, Args&&... args) 
  -> std::future<typename std::result_of<F (Args...)>::type>
{
  using return_type = typename std::result_of<F(Args...)>::type;

  auto task = std::make_shared<std::packaged_task<return_type ()>>
  (
	 std::bind(std::forward<F>(f), std::forward<Args>(args)...)
  );
	
  std::future<return_type> result = task->get_future();

  { // Critical section
	 Lock lock (queue_mutex);

	 // Don't allow an enqueue after stopping
	 if (stop)
		throw std::runtime_error ("enqueue on stopped ThreadPool");

	 // Push work back on the queue
	 tasks.emplace ([task](){ (*task)(); });
  } // End critical section

  // Notify a thread that there is new work to perform
  condition.notify_one ();
  return result;
}

// Add a new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueueAndDetach (F&& f, Args&&... args) 
  -> void
{
  { // Critical section
	 Lock lock (queue_mutex);

	 // Don't allow an enqueue after stopping
	 if (stop)
		throw std::runtime_error ("enqueue on stopped ThreadPool");

	 // Push work back on the queue
	 tasks.emplace (std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  } // End critical section

  // Notify a thread that there is new work to perform
  condition.notify_one ();
}

// Destructor joins all worker threads
ThreadPool::~ThreadPool ()
{
  { // Critical section
	 Lock lock (queue_mutex);
	 stop = true;
  } // End critical section

  condition.notify_all ();

  // Wait for threads to complete work
  for (std::thread &worker : workers)
  {
	 if (worker.joinable ()) { worker.join(); }
  }
}
