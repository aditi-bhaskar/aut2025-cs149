## Assignment 2
#### Sally Zhu (salzhu) & Aditi Bhaskar (aditijb)

#### Describe your task system implementation (1 page is fine). In additional to a general description of how it works, please make sure you address the following questions:
1. How did you decide to manage threads? (e.g., did you implement a thread pool?)
2. How does your system assign tasks to worker threads? Did you use static or dynamic assignment?
3. How did you track dependencies in Part B to ensure correct execution of task graphs?

Our final part A system used a dynamic assignment of tasks to worker threads and coordinated task assignment using an atomic variable `cur_task`.

TODO based on part b!

#### In Part A, you may have noticed that simpler task system implementations (e.g., a completely serial implementation, or the spawn threads every launch implementation), perform as well as or sometimes better than the more advanced implementations. Please explain why this is the case, citing certain tests as examples. For example, in what situations did the sequential task system implementation perform best? Why? In what situations did the spawn-every-launch implementation perform as well as the more advanced parallel implementations that use a thread pool? When does it not?

#### Describe one test that you implemented for this assignment. What does the test do, what is it meant to check, and how did you verify that your solution to the assignment did well on your test? Did the result of the test you added cause you to change your assignment implementation?

The test adds 1 to 10000 array elements, and batches this into tasks of size 1-element with a small sleep, except for one task which processes a large chunk of the elements. We see that the parallelized task pool implementation does much better.

TODO elaborate + add picture
