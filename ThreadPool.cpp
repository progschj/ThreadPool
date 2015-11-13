// Purpose: Thread pool

// Based on https://github.com/progschj/ThreadPool

#include "ThreadPool.hpp"

ThreadPool::ThreadPool (size_t threads)
{
   workers.reserve (threads);

   for (size_t count = 0; count < threads; ++count)
   {
      // Worker execution loop
      workers.emplace_back ([this]()
      {
         for (;;)
         {
            // Task to execute
            std::function<void ()> task;

            // Wait for additional work signal
            { // Critical section
               // Wait to be notified of work
               Lock lock (this->queue_mutex);
               this->condition.wait (lock, [this]()
               { 
                  return this->stop || !this->tasks.empty (); 
               });

               // If stopping and no work remains, exit the work loop and thread
               if (this->stop && this->tasks.empty ())
                  break;

               // Dequeue the next task
               task = std::move (this->tasks.front ());
               this->tasks.pop ();
            } // End critical section

            // Execute
            task ();
         }
      });
   }
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
