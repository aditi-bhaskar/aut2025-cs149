## Assignment 2
#### Sally Zhu (salzhu) & Aditi Bhaskar (aditijb)

#### Describe your task system implementation (1 page is fine). In additional to a general description of how it works, please make sure you address the following questions:
1. How did you decide to manage threads? (e.g., did you implement a thread pool?)
2. How does your system assign tasks to worker threads? Did you use static or dynamic assignment?
3. How did you track dependencies in Part B to ensure correct execution of task graphs?

We used a thread pool to dynamically assign threads in both parts A (spin and sleep) and B. These spawned threads were all workers, and if we were given `num_threads = 8`, we created 7 worker threads (besides our main thread) to execute a function which spun forever. 

In part A (spawn) we statically assigned each thread to execute a consecutive set of tasks. For the remainder of part A we used dynamic assignment for which we maintained a `current_task_index` variable to indicate which task was the next free task. For part B, we used a task pool (implemented as a vector) and popped off the next ready task. 

In part B, we tracked dependencies between launches, where we define one launch to include a set of tasks which may be run concurrently. We created structs for each launch, in which we stored the number of parents a launch had, and the number of children a launch had. All launches were stored in a map. With this information, upon receiving a new launch, we can update all of the launch's children and parent launches to ensure dependencies are maintained. We operate under the provided assumption that, if a launch A has a dependency on launch B, then launch B must be given to the `runAsyncWithDeps()` function before launch A.

In order to track when a launch (and its tasks) can run, we maintain and decrement the parent launches, and the number of tasks, remaining for each launch. This way, once a launch A has zero parent launches that are still pending/running, launch A can be run. Similarly, as soon as all tasks in launch A are complete, all of launch A's children can be notified that they have one less parent to wait on!

By maintaining a thread pool and a task pool in part B, we made it possible to run multiple tasks concurrently in the case that each launch could not (independently) make use of all the cores. (For instance, we have 8 cores and 3 launches with one task each; barring interdependencies, these tasks can be run concurrently). Maintaining the task pool also helped make the correctness of the code more straightforward -- initially, we found that running one launch after another required a lot of extra state and the code maintainability became difficult.

We describe some more details for each method below. 

`TaskSystemParallelSpawn`: Thread `i` is assigned task `i`, `i+n_threads`, i+2n_threads` and so on in a helper function, and each task is executed sequentially by the thread. We join all threads at the end. 

`TaskSystemParallelThreadPoolSpinning`: We have a helper function `spinRunThread` that contains a `while(true)` loop. We keep track of the total number of tasks that have been completed as an atomic variable. When a thread grabs a new task, it updates this counter which we then use to check whether we can kill the threads. We kill the threads in the destructor when all tasks have been grabbed and each thread has finished executing and finally join the threads. 

`TaskSystemParallelThreadPoolSleeping`: We adapt the prior method using a condition variable. When there are no more tasks to execute and a thread is waiting for other threads to finish, we use `cv.wait()` so the threads can sleep. 

**async** `TaskSystemParallelThreadPoolSleeping`: Some more detail about how we track the dependencies is that every launch has a list of its children (what depends on it) and the number of parents. For each launch, we track how many remaining tasks there are. We also keep a task queue where each task contains a runnable function and its index. When a launch fully finishes (all the tasks are done), e.g. when its remaining count is zero, for each of its children, we decrease their `num_parents`. When a child has `num_parents == 0`, we can launch it and add all its tasks to the task queue (or when the run async function is called on a launch with no dependencies). Otherwise our worker function `sleepRunThread` is very similar to `TaskSystemParallelThreadPoolSleeping`. We know that we are done when the number of launches remaining (which we track) is zero and the task queue is empty. 

The `sync()` function contains a while loop that continues until `n_launches_left` is 0 and the task queue is empty. 

#### In Part A, you may have noticed that simpler task system implementations (e.g., a completely serial implementation, or the spawn threads every launch implementation), perform as well as or sometimes better than the more advanced implementations. Please explain why this is the case, citing certain tests as examples. For example, in what situations did the sequential task system implementation perform best? Why? In what situations did the spawn-every-launch implementation perform as well as the more advanced parallel implementations that use a thread pool? When does it not?

The following results are written for 8 threads (1 + 7 worker threads) on AWS.

For `super_super_light`, the serial implementation outperforms the spawn, spin, and sleep. This is because the test case completes data-copy tasks, which are quite small, so the bulk of the overhead comes from creating threads. We see in both our timing results and the results that Always Spawn is about 14x as slow as serial since new threads are spawned for each of the 400 tasks, and that creating each thread only once in Spin and Sleep gives a much slower, 2.5x runtime. 

With `super_light`, the serial implementation (about 87ms) is about 6x slower than the two thread pool methods (about 13-14ms). This might be because there is more work being done in each task (the increment and the copy), and batching these sequential operations across threads gives greater benefit than the overhead of creating the threads. Always spawn is still over 1.5x the runtime, which means spawning a thread for every task still creates a huge overhead. 

With `recursive_fibonacci` and `spin_between_run_calls`, we see the `spawn_every_launch` implementation approach the runtime of the two thread pool methods. In `mandelbrot_chunked`, we see `spin_between_runs` performs 2ms faster than the thread pool methods. This is because of the intensity of the computations, which makes the overhead of creating threads much less than the overhead of orchestrating the thread pool. 

In some other tests, like `ping_pong_equal`, and both `math_operations_*`, we see the runtime of thread pool spin is less than thread pool sleep by about 10ms. We see this trend because sleeping often comes with an overhead of the condition variables.

#### Describe one test that you implemented for this assignment. What does the test do, what is it meant to check, and how did you verify that your solution to the assignment did well on your test? Did the result of the test you added cause you to change your assignment implementation?

We created a test which adds 1 to 10000 array elements, and batches this into tasks of size 1-element with a small sleep, except for one task which processes a large chunk of the elements (all the remaining elements).

We tested correctness and runtime when the work was distributed unevenly, and was meant to especially test our work-stealing capabilities. Note that our `Always_Spawn` was implemented with static assignment, so it took longer than the thread pool methods -- likely because the threads didn't steal work! Our test result did not cause us to change our assignment implementation.

We see that the parallelized task pool implementation does much better --- because threads can grab new tasks when they are finished (in this case this would be the single-element tasks). Here are our performance results on Myth:

```
Sync, 8 threads ===================================================
[Serial]:                                       [1053.168] ms
[Parallel + Always Spawn]:                      [247.655] ms
[Parallel + Thread Pool + Spin]:                [148.665] ms
[Parallel + Thread Pool + Sleep]:               [139.302] ms

Sync, 16 threads ==================================================
[Serial]:                                       [1100.544] ms
[Parallel + Always Spawn]:                      [151.950] ms
[Parallel + Thread Pool + Spin]:                [118.641] ms
[Parallel + Thread Pool + Sleep]:               [134.640] ms

Async, 8 threads =================================================
[Serial]:                                       [1236.144] ms
[Parallel + Thread Pool + Sleep]:               [243.009] ms

Async, 16 threads =================================================
[Serial]:                                       [1241.754] ms
[Parallel + Thread Pool + Sleep]:               [159.105] ms
```
