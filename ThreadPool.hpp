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
      // Launches specified number of worker threads
      ThreadPool (size_t threads);
      ~ThreadPool ();

      // Not copyable
      ThreadPool (const ThreadPool &) = delete;
      ThreadPool& operator= (const ThreadPool &) = delete;

      // Not moveable
      ThreadPool (ThreadPool &&) = delete;
      ThreadPool& operator= (const ThreadPool &&) = delete;

      // Enqueue task and return std::future<>
      template<class Callable, class... Args>
      auto enqueue (Callable&& callable, Args&&... args) 
         -> std::future<typename std::result_of<Callable (Args...)>::type>;

      // Enqueue task without requiring capture of std::future<>
      // Note: Best not to let exceptions escape the callable
      template<class Callable, class... Args>
      auto enqueueAndDetach (Callable&& callable, Args&&... args) 
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

// Add a new work item to the pool
template<class Callable, class... Args>
auto ThreadPool::enqueue (Callable&& callable, Args&&... args) 
   -> std::future<typename std::result_of<Callable (Args...)>::type>
{
   using return_t = typename std::result_of<Callable (Args...)>::type;
   using task_t = std::packaged_task<return_t ()>;

   auto task = std::make_shared<task_t> (std::bind (std::forward<Callable> (callable), std::forward<Args> (args)...));
   std::future<return_t> result = task->get_future();

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
template<class Callable, class... Args>
auto ThreadPool::enqueueAndDetach (Callable&& callable, Args&&... args) 
   -> void
{
   { // Critical section
      Lock lock (queue_mutex);

      // Don't allow an enqueue after stopping
      if (stop)
         throw std::runtime_error ("enqueue on stopped ThreadPool");

      // Push work back on the queue
      tasks.emplace (std::bind (std::forward<Callable> (callable), std::forward<Args> (args)...));
   } // End critical section

   // Notify a thread that there is new work to perform
   condition.notify_one ();
}

