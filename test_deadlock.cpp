#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <atomic>
#include <chrono>

int main() {
    // 测试场景1：正常析构（无任务）
    {
        ThreadPool pool(4);
        // 立即析构，无任务执行
    }
    std::cout << "Test 1 passed: No deadlock when destroying empty pool." << std::endl;

    // 测试场景2：提交任务后析构
    {
        ThreadPool pool(4);
        std::atomic<int> counter(0);
        
        // 提交大量短任务
        for (int i = 0; i < 1000; ++i) {
            pool.enqueue([&counter] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }
        
        // 不等待任务完成，直接析构
    }
    std::cout << "Test 2 passed: No deadlock when destroying pool with pending tasks." << std::endl;

    // 测试场景3：提交耗时任务后析构
    {
        ThreadPool pool(4);
        
        // 提交一个长时间运行的任务
        pool.enqueue([] {
            std::this_thread::sleep_for(std::chrono::seconds(2));
        });
        
        // 立即析构（任务可能仍在执行）
    }
    std::cout << "Test 3 passed: No deadlock when destroying pool with running tasks." << std::endl;

    std::cout << "All tests passed!" << std::endl;
    return 0;
}