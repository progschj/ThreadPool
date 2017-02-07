#include <array>
#include <future>
#include <string>
#include <vector>

#include "thread_pool.h"

#include "gtest/gtest.h"

TEST(ThreadPool, Constructors) {
    EXPECT_NO_THROW(ThreadPool());
    EXPECT_NO_THROW(ThreadPool(1));
    EXPECT_THROW(ThreadPool(0), std::runtime_error);
}

TEST(ThreadPool, Enqueue) {
    ThreadPool pool;

    ASSERT_NO_THROW(pool.enqueue([](const std::string& i) { return i + " " + i; }, "kitten"));

    ASSERT_NO_THROW(pool.enqueue([](const int& i) { return 12 * i; }, 3));
    ASSERT_NO_THROW(pool.enqueue([](const float& i) { return 0.1f * i; }, 1.38f));
    ASSERT_NO_THROW(pool.enqueue([](const double& i) { return 0.1 * i; }, 1.38));

    auto result_a = pool.enqueue([](const std::string& i) { return i + " " + i; }, "kitten").get();
    EXPECT_EQ(result_a, "kitten kitten");

    auto result_b = pool.enqueue([](const int& i) { return 12 * i; }, 3).get();
    EXPECT_EQ(result_b, 36);

    auto result_c = pool.enqueue([](const float& i) { return 0.1f * i; }, 1.38f).get();
    EXPECT_NEAR(result_c, 0.138f, 1E-4);

    auto result_d = pool.enqueue([](const double& i) { return 0.1 * i; }, 1.38).get();
    EXPECT_NEAR(result_d, 0.138, 1E-4);
}

TEST(ThreadPool, EnqueueExample) {
    ThreadPool pool;

    std::vector<std::future<int>> results_int;
    results_int.reserve(12);
    for (auto i = 0; i < 12; ++i) {
        results_int.emplace_back(pool.enqueue([](int i) { return i * i; }, i));
    }

    auto sum = 0;
    for (auto&& result : results_int) {
        sum += result.get();
    }

    EXPECT_EQ(sum, 506);

    EXPECT_NO_THROW(pool.enqueue([](const std::string& i) { return "the " + i + " says hello"; }, "cat"));
}