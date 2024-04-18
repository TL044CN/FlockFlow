/**
 * @file ThreadPool.hpp
 * @author TL044CN
 * @brief Threadpool class for multithreading purposes
 * @version 0.2
 * @date 2024-01-21
 *
 * @copyright Copyright (c) TL044CN 2024
 *
 */
#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstdint>

namespace FlockFlow {

/**
 * @brief Manages a number of Threads to do Tasks in parallel (using true multithreading)
 */
class ThreadPool {
    template<typename Func>
#if __cpp_lib_is_invocable
    using resultType = std::invoke_result<Func&&>::type;
#else
    using resultType = std::result_of<Func&&>::type;
#endif
private:
    /**
     * @brief Task in the priority_queue
     */
    struct Task {
        using Package = std::packaged_task<void()>;
        mutable Package mTask;
        uint32_t mPriority;

        /**
         * @brief Construct a new Task object
         *
         * @param task the task to do / function to execute
         * @param priority the Priority of the task
         */
        Task(Package&& task, uint32_t priority);

        /**
         * @brief Compare the priority of two Tasks
         *
         * @param other the other Task to compare to
         * @return auto the result of the comparison
         */
        [[nodiscard]] std::strong_ordering operator <=> (const Task& other) const;
    };

private:
    std::vector<std::thread> mThreads;
    std::priority_queue<Task, std::vector<Task>, std::less<Task>> mTasks;

    std::mutex mQueueMutex;
    std::condition_variable mThreadPoolConditional;

    std::atomic<bool> mTerminate = false;
    std::atomic<uint32_t> mIdleThreads;
    std::atomic_flag mPause;

public:
    /**
     * @brief Create and start new Thread Pool with the given amount of Threads
     *
     * @param threadpoolSize number of Threads, defaults to the number of Hardware threads
     */
    explicit ThreadPool(const uint32_t threadpoolSize = std::thread::hardware_concurrency());

    /**
     * @brief Cleans up the Threads of the Threadpool
     */
    ~ThreadPool();

public:
    /**
     * @brief                       Add a new Task to the Thread Pool
     *
     * 
     * @tparam                      Func Type of the Function to queue
     * @tparam                      Ret Result type of the Function to queue
     * 
     * @param function              the Function to queue
     * @param priority              the Priority of the task execution.
     *                              higher numbers result in higher priority.
     * 
     * @return std::future<Ret>     Future, holding the returned value of the Function
     */
    template <typename Func, typename Ret = resultType<Func>>
    [[nodiscard]] std::future<Ret> queueJob(Func&& function, uint32_t priority = 0) {
        auto task = std::packaged_task<Ret()>(std::forward<Func>(function));
        auto future = task.get_future();
        {
            std::lock_guard<std::mutex> lock(mQueueMutex);
            mTasks.emplace(Task(Task::Package(std::move(task)), priority));
        }

        mThreadPoolConditional.notify_one();
        return future;
    }

    /**
     * @brief Pauses the execution of new Tasks
     */
    void pause();

    /**
     * @brief Resumes the execution of new Tasks
     */
    void resume();

    /**
     * @brief Check weather the Threadpool is currently accepting new tasks or not.
     * 
     * @return true the Threadpool is paused
     * @return false the Threadpool is not paused and executes new tasks
     */
    [[nodiscard]] bool isPaused() const ;

    /**
     * @brief Returns the number of idle Threads
     *
     * @return uint32_t the number of idle threads
     */
    uint32_t idleThreads() const;

    /**
     * @brief Returns the number of Threads available to the Threadpool
     * 
     * @return const uint32_t 
     */
    const uint32_t maxThreads() const;

    /**
     * @brief Returns true when the ThreadPool has active tasks
     *
     * @return true the Threadpool has active tasks
     * @return false the Threadpool is idle
     */
    bool hasTasks();

private:
    /**
     * @brief Handles the execution of Tasks
     */
    void taskManagementLoop();

};

}
