#include <ThreadPool.hpp>

namespace FlockFlow {

ThreadPool::Task::Task(Task::Package&& task, uint32_t priority)
    : mTask(std::move(task)), mPriority(priority) {}

std::strong_ordering ThreadPool::Task::operator<=>(const ThreadPool::Task& other) const {
    return mPriority <=> other.mPriority;
}


ThreadPool::ThreadPool(const uint32_t threadpoolSize)
    : mIdleThreads(threadpoolSize) {
    for ( uint32_t num = threadpoolSize; num--;)
        mThreads.emplace_back([this] { taskManagementLoop(); });
}

ThreadPool::~ThreadPool() {
    mTerminate = true;
    mThreadPoolConditional.notify_all();
    for ( auto& thread : mThreads )
        thread.join();
    mThreads.clear();
}

void ThreadPool::pause() {
    mPause.test_and_set();
}

void ThreadPool::resume() {
    mPause.clear();
    mPause.notify_all();
}

bool ThreadPool::isPaused() const {
    return mPause.test();
}

uint32_t ThreadPool::idleThreads() const {
    return mIdleThreads;
}

const uint32_t ThreadPool::maxThreads() const {
    return mThreads.size();
}

bool ThreadPool::hasTasks() {
    bool empty;
    {
        std::lock_guard<std::mutex> loock(mQueueMutex);
        empty = mTasks.empty();
    }
    return !empty;
}

void ThreadPool::taskManagementLoop() {
    while ( true ) {

        if ( mPause.test() )
            mPause.wait(true);

        std::unique_lock<std::mutex> lock(mQueueMutex);
        mThreadPoolConditional.wait(lock, [this] { return !mTasks.empty() || mTerminate; });
        if ( mTerminate ) break;


         if (mPause.test()){
            lock.unlock();
            mPause.wait(true);
            lock.lock();
        }

        if (!mTasks.empty()) {
            --mIdleThreads;
        
            Task::Package currentTask = std::move(mTasks.top().mTask);
            mTasks.pop();
            lock.unlock();
            currentTask();

            ++mIdleThreads;
        }
    }
}

}