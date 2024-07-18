#include "catch2/catch.hpp"
#include "FlockFlow.hpp"
#include <string>
#include <future>

TEST_CASE("ThreadPool FizzBuzz", "[FizzBuzz]") {
    FlockFlow::ThreadPool pool;
    SECTION("TestFizzBuzz") {
        std::vector<std::future<std::string>> futures;

        // Queue tasks to solve the FizzBuzz problem
        for ( int i = 1; i <= 100; ++i ) {
            futures.push_back(pool.queueJob(
                [i]() -> std::string {
                    if ( i % 15 == 0 ) {
                        return "FizzBuzz";
                    } else if ( i % 3 == 0 ) {
                        return "Fizz";
                    } else if ( i % 5 == 0 ) {
                        return "Buzz";
                    } else {
                        return std::to_string(i);
                    }
                }
            ));
        }

        // Check if the results are correct
        for ( int i = 0; i < 100; ++i ) {
            std::string result = futures[i].get();
            if ( (i + 1) % 15 == 0 ) {
                REQUIRE(result == "FizzBuzz");
            } else if ( (i + 1) % 3 == 0 ) {
                REQUIRE(result == "Fizz");
            } else if ( (i + 1) % 5 == 0 ) {
                REQUIRE(result == "Buzz");
            } else {
                REQUIRE(result == std::to_string(i + 1));
            }
        }
    }
}