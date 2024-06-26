#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "FlockFlow.hpp"
#include <future>
#include <chrono>
#include <condition_variable>

using namespace FlockFlow;

class FutureTimeout : public std::runtime_error {
public:
    FutureTimeout(): std::runtime_error("Future did not become ready within the specified timeout") {}
};

template <typename T>
T get_for(std::future<T>& future, const std::chrono::seconds& timeout) {
    if ( future.wait_for(timeout) == std::future_status::timeout ) {
        throw FutureTimeout();
    }
    return future.get();
}

template <typename T>
void check_timeout(std::future<T>& future, const std::chrono::seconds& timeout) {
    if ( future.wait_for(timeout) == std::future_status::timeout ) {
        throw FutureTimeout();
    }
    return;
}

TEST_CASE("FlockFlow tests", "[ThreadPool]") {
    ThreadPool pool;

    SECTION("TestQueueJob") {
        auto future = pool.queueJob([]() { return 42; });
        REQUIRE(future.get() == 42);
    }

    SECTION("TestPauseAndResume") {
        pool.pause();
        REQUIRE(pool.isPaused());

        auto future = pool.queueJob([]() { return 42; });
        pool.resume(); // resume the ThreadPool before calling get_for
        REQUIRE_NOTHROW(check_timeout(future, std::chrono::seconds(1))); // check if task is not executed within 1 second

        REQUIRE_FALSE(pool.isPaused());
        REQUIRE(future.get() == 42); // now the task should be executed
    }


    SECTION("TestIdleThreads") {
        REQUIRE(pool.idleThreads() == pool.maxThreads()); // initially all threads should be idle

        auto future = pool.queueJob([]() { std::this_thread::sleep_for(std::chrono::seconds(2)); return 42; });
        REQUIRE_THROWS_AS(get_for(future, std::chrono::seconds(1)), FutureTimeout); // check if task is not executed within 1 second

        future.get(); // wait for the task to finish
        REQUIRE(pool.idleThreads() == pool.maxThreads()); // all threads should be idle again
    }

    SECTION("TestHasTasks") {
        REQUIRE_FALSE(pool.hasTasks()); // initially there should be no tasks

        pool.pause();

        auto future = pool.queueJob([]() { std::this_thread::sleep_for(std::chrono::seconds(2)); return 42; });
        REQUIRE(pool.hasTasks()); // now there should be a task

        pool.resume();

        REQUIRE_THROWS_AS(get_for(future, std::chrono::seconds(1)), FutureTimeout); // check if task is not executed within 1 second

        future.get(); // wait for the task to finish
        REQUIRE_FALSE(pool.hasTasks()); // there should be no tasks again
    }

    SECTION("TestPriorityQueue") {
        std::vector<int> lowPriorityClock(pool.maxThreads(), 0);
        std::vector<int> highPriorityClock(pool.maxThreads(), 0);
        std::vector<std::future<void>> futures;
        std::mutex m;

        pool.pause();

        futures.push_back(pool.queueJob(
            [&lowPriorityClock, &m, &pool]() {
                std::lock_guard<std::mutex> lock(m);
                for ( int i = 0; i < pool.maxThreads(); ++i ) {
                    lowPriorityClock[i]++;
                }
            }, 1)
        );

        for ( int i = 0; i < pool.idleThreads(); ++i ) {
            futures.push_back(pool.queueJob(
                [&highPriorityClock, &m, &pool, i]() {
                    std::lock_guard<std::mutex> lock(m);
                    highPriorityClock[i]++;
                }, 2)
            );
        }

        pool.resume();

        // Give some time for the tasks to execute
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for ( auto& future : futures ) {
            future.get();
        }

        // Check if the low priority task started last
        for ( int i = 0; i < pool.maxThreads(); ++i ) {
            REQUIRE(lowPriorityClock[i] <= highPriorityClock[i]);
        }
    }
}