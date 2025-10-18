#include "tasksys.h"
#include <iostream>
#include <unistd.h>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    n_threads = num_threads; 
    workers = new std::thread[num_threads];

    for (int j = 1; j < n_threads; j++) {
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSleeping::sleepRunThread, this, j);
    }

}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {

    this->sync();

    threads_ready_to_die = 0;
    killing_threads = true;
    while(n_launches_left != 0 || threads_ready_to_die < n_threads-1) { 
        cv.notify_all();
    }

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    this->sync();

    std::vector<TaskID> nodep = {};
    this->runAsyncWithDeps(runnable, num_total_tasks, nodep);

    this->sync();

}

void TaskSystemParallelThreadPoolSleeping::addToTaskQueue(LaunchInfo* launch) {

    for(size_t i = 0; i < launch->remaining_tasks; i++) {
        TaskInfo* new_task_info = new TaskInfo(launch->id, launch->remaining_tasks, i, launch->task_runnable);
        task_queue.push_back(*new_task_info); 
    }
}

void TaskSystemParallelThreadPoolSleeping::sleepRunThread(int thread_id) {

    while(true) {

        std::unique_lock<std::mutex> lk(taskMutex);

        if (n_launches_left <= 0 && task_queue.empty() && killing_threads) {
            threads_ready_to_die += 1;
            return; 
        }

        while (task_queue.empty()) {
            cv.wait(lk);
            if (n_launches_left <= 0 && task_queue.empty() && killing_threads) { // aditi
                threads_ready_to_die += 1;
                return;   
            }
        }


        // grab new task 
        TaskInfo task = task_queue.back(); 
        task_queue.pop_back();
        lk.unlock();
        
        task.task_runnable->runTask(task.cur_id, task.n_total_tasks);

        lk.lock(); 
        launches[task.id]->remaining_tasks--; 

        if (launches[task.id]->remaining_tasks <= 0) { // aditi check
            std::vector<TaskID> children_list = launches[task.id]->children;
            for (size_t i = 0; i < children_list.size(); i++) {
                launches[children_list[i]]->n_parents--; 
                if (launches[children_list[i]]->n_parents == 0) {
                    // add child to task queue 
                    this->addToTaskQueue(launches[children_list[i]]); 
                }
            }

            n_launches_left--; 
            lk.unlock();
            cv.notify_all();
        } 
        else {
            lk.unlock();
        }
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {
    taskMutex.lock(); 

    int launch_id = max_launch_id++;
    n_launches_left++; 
    std::vector<TaskID> emptyChildren;

    LaunchInfo *launch_info = new LaunchInfo(
        launch_id, num_total_tasks, deps.size(), emptyChildren, runnable
    );

    // add this launch to parents' structs
    launches[launch_id] = launch_info; 

    int deps_left = deps.size(); 

    for (size_t i = 0; i < deps.size(); i++) {
        launches[deps[i]]->children.push_back(launch_id);

        if (launches[deps[i]]->remaining_tasks == 0) {
            deps_left--; 
            launch_info->n_parents--; 
        }
    }

    launch_info->n_parents = deps_left;
    launches[launch_id] = launch_info;
    
    // if the launch has no dependencies, add to task queue 
    if (deps_left == 0) {
        this->addToTaskQueue(launch_info); 
        taskMutex.unlock();  // try flipping order of unlock/notify all to reduce runtime
        cv.notify_all(); 
    } else {
        taskMutex.unlock(); // make sure to only unlock mutex once.
    }

    return launch_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    taskMutex.lock();
    bool can_finish = (!task_queue.empty()) || n_launches_left > 0;
    taskMutex.unlock();

    while(can_finish) {
        taskMutex.lock();
        can_finish = (!task_queue.empty()) || n_launches_left > 0;
        taskMutex.unlock();
    }

    return;
}

