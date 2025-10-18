## Assignment 2
#### Sally Zhu (salzhu) & Aditi Bhaskar (aditijb)

#### Describe your task system implementation (1 page is fine). In additional to a general description of how it works, please make sure you address the following questions:
1. How did you decide to manage threads? (e.g., did you implement a thread pool?)
2. How does your system assign tasks to worker threads? Did you use static or dynamic assignment?
3. How did you track dependencies in Part B to ensure correct execution of task graphs?

We used a thread pool to dynamically assign threads in both parts A (spin and sleep) and B. These spawned threads were all workers, and if we were given num_threads = 8, we created 7 worker threads (besides our main thread) to execute a function which spun forever. 

In part A (spawn) we statically assigned each thread to execute a consecutive set of tasks. For the remainder of part A we used dynamic assignment for which we maintained a `current_task_index` variable to indicate which task was the next free task. For part B, we used a task pool (implemented as a vector) and popped off the next ready task. 

In part B, we tracked dependencies between launches, where we define one launch to include a set of tasks which may be run concurrently. We created structs for each launch, in which we stored the number of parents a launch had, and the number of children a launch had. All launches were stored in a map. With this information, upon receiving a new launch, we can update all of the launch's children and parent launches to ensure dependencies are maintained. We operate under the provided assumption that, if a launch A has a dependency on launch B, then launch B must be given to the runAsyncWithDeps() function before launch A.

In order to track when a launch (and its tasks) can run, we maintain and decrement the parent launches, and the number of tasks, remaining for each launch. This way, once a launch A has zero parent launches that are still pending/running, launch A can be run. Similarly, as soon as all tasks in launch A are complete, all of launch A's children can be notified that they have one less parent to wait on!

By maintaining a thread pool and a task pool in part B, we made it possible to run multiple tasks concurrently in the case that each launch could not (independently) make use of all the cores. (For instance, we have 8 cores and 3 launches with one task each; barring interdependencies, these tasks can be run concurrently). Maintaining the task pool also helped make the correctness of the code more straightforward -- initially, we found that running one launch after another required a lot of extra state and the code maintainability became difficult.

#### In Part A, you may have noticed that simpler task system implementations (e.g., a completely serial implementation, or the spawn threads every launch implementation), perform as well as or sometimes better than the more advanced implementations. Please explain why this is the case, citing certain tests as examples. For example, in what situations did the sequential task system implementation perform best? Why? In what situations did the spawn-every-launch implementation perform as well as the more advanced parallel implementations that use a thread pool? When does it not?



#### Describe one test that you implemented for this assignment. What does the test do, what is it meant to check, and how did you verify that your solution to the assignment did well on your test? Did the result of the test you added cause you to change your assignment implementation?

We created a test which adds 1 to 10000 array elements, and batches this into tasks of size 1-element with a small sleep, except for one task which processes a large chunk of the elements. We see that the parallelized task pool implementation does much better.

Here are our performance results on Myth:

Async, 8 threads =================================================
[Serial]:                                       [1236.144] ms
[Parallel + Thread Pool + Sleep]:               [243.009] ms

Async, 16 threads =================================================
[Serial]:                                       [1241.754] ms
[Parallel + Thread Pool + Sleep]:               [159.105] ms

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

