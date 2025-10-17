#include "tasksys.h"
#include <iostream>

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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    std::cout << "entering constructor" << std::endl;

    n_threads = num_threads; 
    workers = new std::thread[num_threads];

    for (int j = 1; j < n_threads; j++) {
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSleeping::sleepRunThread, this, j);
    }

    std::cout << "exiting constructor" << std::endl;
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    std::cout << "entering destructor" << std::endl;

    this->sync();

    // cv.notify_all();
    std::cout << "161 finished sync in destructor | " << n_launches_left << std::endl;

    threads_ready_to_die = 0;
    killing_threads = true;
    while(n_launches_left != 0 || threads_ready_to_die < n_threads-1) { // aditi check
        std::cout << "in destructor | n_launches_left " << n_launches_left << " | task_queue_empty " << task_queue.empty() << 
        " | threads_ready_to_die " << threads_ready_to_die << " | n_threads-1 " << n_threads-1 << std::endl;
        cv.notify_all();
    }
    
    std::cout << "in destructor | right before joining all threads together" << " | threads_ready_to_die " << threads_ready_to_die << " | n_threads-1 " << n_threads-1 << std::endl;

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }

    std::cout << "exiting destructor" << std::endl;

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // std::cout << "entering run" << std::endl;

    this->sync();

    std::vector<TaskID> nodep = {};
    this->runAsyncWithDeps(runnable, num_total_tasks, nodep);

    this->sync();

    // std::cout << "exiting run" << std::endl;

}

void TaskSystemParallelThreadPoolSleeping::addToTaskQueue(LaunchInfo* launch) {

    // std::cout << "entering addToTaskQueue" << std::endl;

    for(size_t i = 0; i < launch->remaining_tasks; i++) {
        // std::cout << "203 " << launch->id << std::endl;
        TaskInfo* new_task_info = new TaskInfo(launch->id, launch->remaining_tasks, i, launch->task_runnable);
        task_queue.push_back(*new_task_info); 
    }
}

void TaskSystemParallelThreadPoolSleeping::sleepRunThread(int thread_id) {

    // std::cout << "entering sleepRunThread" << std::endl;

    while(true) {

        std::unique_lock<std::mutex> lk(taskMutex);

        if (n_launches_left <= 0 && task_queue.empty() && killing_threads) {
        // if (n_launches_left <= 0 && task_queue.empty()) {
            threads_ready_to_die += 1;
            std::cout << "exiting sleepRunThread: v1" << std::endl;
            return; 
        }

        while (task_queue.empty()) {
            std::cout << "spinning 222" << std::endl;
            cv.wait(lk);
            if (n_launches_left <= 0 && task_queue.empty() && killing_threads) { // aditi
            // if (n_launches_left <= 0 && task_queue.empty()) { // aditi
                threads_ready_to_die += 1;
                std::cout << "exiting sleepRunThread: v2" << std::endl;
                return;   
            }
        }


        // grab new task 
        TaskInfo task = task_queue.back(); 
        task_queue.pop_back();
        lk.unlock();
        
        std::cout << "running runnable 241 | launch " << task.id << " | task " << task.cur_id << " | thread " << thread_id << " | n_launches_left " << n_launches_left << std::endl;

        task.task_runnable->runTask(task.cur_id, task.n_total_tasks);

        std::cout << "done running | launch " << task.id << " | task " << task.cur_id << " | thread " << thread_id << " | n_launches_left " << n_launches_left << std::endl;

        // launchMutex.lock();
        lk.lock(); 
        launches[task.id]->remaining_tasks--; 

        std::cout << "update | launch " << task.id << " | remaining " << launches[task.id]->remaining_tasks << " | n_launches_left " << n_launches_left << std::endl;

        if (launches[task.id]->remaining_tasks == 0) { // aditi check
            std::vector<TaskID> children_list = launches[task.id]->children;
            for (size_t i = 0; i < children_list.size(); i++) {
                launches[children_list[i]]->n_parents--; 
                if (launches[children_list[i]]->n_parents == 0) {
                    // add child to task queue 
                    this->addToTaskQueue(launches[children_list[i]]); 
                }
            }

            n_launches_left--; 
            std::cout << "decrementing " << n_launches_left << std::endl;
            lk.unlock();

            cv.notify_all();
        } 
        else {
            lk.unlock();
        }
        // lk.unlock();
    }

    std::cout << "exiting sleepRunThread" << std::endl;
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {
    //
    // TODO: CS149 students will implement this method in Part B.
    //
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
        taskMutex.unlock(); 
        cv.notify_all(); 
        // taskMutex.lock(); 
    } else {
        taskMutex.unlock(); // make sure to only unlock mutex once.
    }


    // if (n_launches_left == 1) { // n_launches_left *was* zero, we just pushed a new launch
    //     cv.notify_all(); 
    // }

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    std::cout << "entering sync" << std::endl;

    while(!task_queue.empty() || n_launches_left > 0) {
        // std::cout << "spinning 326" << std::endl;
        // aditi todo
        std::cout << "in sync | n_launches_left " << n_launches_left << " | task_queue_empty " << task_queue.empty() << std::endl;
    }

    std::cout << "exiting sync | n_launches_left " << n_launches_left << " | task_queue_empty " << task_queue.empty() << std::endl;

    return;
}


// lingering issues:
//   1. when we get to in_sync and there are 2 launches left, but task queue is empty. suggests we dont add tasks onto queue in some situations, which is problematic
//   2. [FIXED] we try to kill the threads before all are ready to die! 