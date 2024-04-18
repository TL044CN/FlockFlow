# Flock Flow

<a href='https://jenkins.shodan.fyi/job/FlockFlow/'><img src='https://jenkins.shodan.fyi/buildStatus/icon?job=FlockFlow'></a>
This small library is supposed to make parallel processing and multithreading effortless.

## Usage:

Create a Threadpool

```c++
// use the maximum hardware concurrency number of threads:
ThreadPool pool;

// ... or specify a number of threads:
ThreadPool pool2(4);    // 4 threads
```

Now assign some Tasks to the Threadpool:

```c++
// queue a function that returns an int and prints something to the console
auto returnFuture = pool.queueJob([] -> int {
    std::cout << "I'm actively doing some work!!" << std::endl;
    return 5;
});
```

Now wait for the Task to be finished and return the result:

```c++
auto vaule = return_future.get();
```

As soon as `.get()` is called, the main thread will block until the value is provided.
All the usual rules of `Futures` apply.


## More Examples:

### Assigning Tasks with Priorities:

You can assign a priority level to each task, and the ThreadPool will execute tasks with higher priority numbers before those with lower ones. Every number can be used, and higher numbers are more important.

```c++
ThreadPool pool(4);    // 4 threads

// queue tasks with priority
auto future1 = pool.queueJob([] {
    std::cout << "Priority 100 task" << std::endl;
}, 100);    // higher priority

auto future2 = pool.queueJob([] {
    std::cout << "Priority 50 task" << std::endl;
}, 50);    // lower priority

auto future3 = pool.queueJob([] {
    std::cout << "Priority 75 task" << std::endl;
}, 75);    // medium priority
```

### Parallel Processing:

The ThreadPool processes all tasks in parallel as long as there are spots in the queue. This means that if you queue up more tasks than there are threads in the ThreadPool, the ThreadPool will start executing the tasks in parallel as soon as a thread becomes available. Tasks will not necessarily finish in the order they were queued.

```c++
ThreadPool pool(4);    // 4 threads

// queue more tasks than there are threads
for (int i = 0; i < 10; ++i) {
    pool.queueJob([i] {
        std::cout << "Task " << i << std::endl;
    });
}
```

### Pausing and Resuming the ThreadPool:

The ThreadPool can be paused and resumed. When the ThreadPool is paused, it will finish the tasks it is currently running but will not start any new tasks. When the ThreadPool is resumed, it will start running new tasks again.

```c++
ThreadPool pool(4);    // 4 threads

// queue some tasks
for (int i = 0; i < 10; ++i) {
    pool.queueJob([i] {
        std::cout << "Task " << i << std::endl;
    });
}

// pause the ThreadPool
pool.pause();

// at this point, the ThreadPool will finish the tasks it is currently running,
// but will not start any new tasks

// resume the ThreadPool
pool.resume();

// now the ThreadPool will start running new tasks again
```