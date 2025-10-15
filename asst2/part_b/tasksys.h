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
    uint64_t num_total_tasks;
    std::vector<TaskID> dependencies;
    IRunnable *task_runnable;

    LaunchInfo(TaskID i, uint64_t n, std::vector<TaskID> d, IRunnable *t):
        id(i), num_total_tasks(n), dependencies(d), task_runnable(t) {}
        
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
        std::atomic<bool> kill_threads{false}; 


        // maintain global task index for processing
        std::atomic<bool> processing_tasks{false};
        std::atomic<int> cur_task_index{0};  // cur task index within the launch
        std::atomic<int> num_threads_done{0}; 
        std::atomic<int> num_threads_awake{0}; 
        std::atomic<int> num_threads_ready_to_die{0}; 

        // per-launch info
        TaskID cur_launch_id;  // cur task id
        std::atomic<int> cur_num_total_tasks{0}; // total tasks for this launch
        IRunnable *cur_runnable{0}; // total tasks for this launch
        
        // maintain DAG info + states for tasks
        std::atomic<int> max_launch_id{0}; // unique launchid for each incoming batched set of tasks
        std::map<TaskID, LaunchInfo*> launches_with_dep;
        std::map<TaskID, LaunchInfo*> ready_to_run;
        std::vector<TaskID> done;

        // used for coordinating/running between threads
        std::mutex myMutex;
        std::condition_variable cv;

        void rebalanceRunning(void);
        void sleepRunThread(int thread_id);
        void grabNewLaunch(void);
};

#endif

