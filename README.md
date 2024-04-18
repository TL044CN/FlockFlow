<a href='https://jenkins.shodan.fyi/job/ThreadPool/'><img src='https://jenkins.shodan.fyi/buildStatus/icon?job=ThreadPool'></a>
# ThreadPool
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
