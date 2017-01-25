//
// Purpose: Simple thread pool
//
// Based on https://github.com/progschj/ThreadPool changes provided as https://github.com/calthron/ThreadPool

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
            { // CRITICAL SECTION
               // Wait to be notified of work
               lock_t lock (queue_mutex);
               condition.wait (lock, [this]()
               { 
                  return stop || !tasks.empty (); 
               });

               // If stopping and no work remains, exit the work loop
               if (stop && tasks.empty ())
                  break;

               // Dequeue the next task
               task.swap (tasks.front ());
               tasks.pop ();
            } // END CRITICAL SECTION

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
      lock_t lock (queue_mutex);
      stop = true;
   } // End critical section

   condition.notify_all ();

   // Wait for threads to complete work
   for (std::thread &worker : workers)
   {
      worker.join();
   }
}
