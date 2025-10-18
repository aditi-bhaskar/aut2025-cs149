#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <map>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

struct LaunchInfo {
    TaskID id;
    uint64_t remaining_tasks;
    uint64_t n_parents;
    std::vector<TaskID> children;
    IRunnable *task_runnable;

    LaunchInfo(TaskID i, uint64_t n, uint64_t p, std::vector<TaskID> d, IRunnable *t):
        id(i), remaining_tasks(n), n_parents(p), children(d), task_runnable(t) {}
        
};

struct TaskInfo {
    TaskID id;
    uint64_t n_total_tasks;
    uint64_t cur_id;
    IRunnable *task_runnable;

    TaskInfo(TaskID i, uint64_t n, uint64_t c, IRunnable *t):
        id(i), n_total_tasks(n), cur_id(c), task_runnable(t) {}
        
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();

    private:
        int n_threads; 
        std::thread* workers;

        std::atomic<int> max_launch_id{0};
        std::atomic<int> n_launches_left{0};

        std::map<TaskID, LaunchInfo*> launches; 
        std::vector<TaskInfo> task_queue; // change to queue for better runtime?

        std::atomic<int> threads_ready_to_die{0};
        std::atomic<bool> killing_threads{false};

        std::mutex taskMutex;

        std::condition_variable cv;

        void addToTaskQueue(LaunchInfo* launch);
        void sleepRunThread(int thread_id);
};

#endif
