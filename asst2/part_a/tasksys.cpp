#include "tasksys.h"
#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>
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
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    n_threads = num_threads;
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::workerHelper(IRunnable* runnable, int num_total_tasks, int thread_id) {
    for(int i = thread_id; i < num_total_tasks; i += n_threads) {
        runnable->runTask(i, num_total_tasks); 
    }
} 

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    std::thread workers[n_threads];

    for (int j = 1; j < n_threads; j++) {
        workers[j] = std::thread(&TaskSystemParallelSpawn::workerHelper, this, runnable, num_total_tasks, j);
    }
    workerHelper(runnable, num_total_tasks, 0); 

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }

}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
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

void TaskSystemParallelThreadPoolSpinning::spinRunThread(int thread_id) {
    while(true) {
        if (cur_task == -1) {
            // std::cout << "120 " << num_threads_done << std::endl;
            // num_threads_done = 0; 
            continue; 
        }
        myMutex.lock(); 
        int my_task = cur_task; 

        if (my_task >= num_total_tasks) {
            myMutex.unlock(); 
            num_threads_done += 1; 
            std::cout << num_threads_done << std::endl;
            while (cur_task != -1) {
                myMutex.lock();
                // std::cout << "129" << " " << cur_task << " " << my_task << " " << num_total_tasks << " " << num_threads_done <<  std::endl;
                myMutex.unlock();
            }
            num_threads_done--; 
            continue; 
        }

        std::cout << "135" << " " << my_task << " " << cur_task << std::endl;

        cur_task += 1; 

        myMutex.unlock(); 

        run_function->runTask(my_task, num_total_tasks);
    }
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    // std::thread workers[num_threads];

    n_threads = num_threads; 
    workers = new std::thread[num_threads];

    for (int j = 1; j < n_threads; j++) {
        std::cout << "starting thread " << j << std::endl;
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSpinning::spinRunThread, this, j);
    }
    
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int n_total_tasks) {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    run_function = runnable; 

    num_threads_done = 0; 
    num_total_tasks = n_total_tasks; 

    cur_task = 0; 
    

    while(num_threads_done < n_threads - 1) {
        // std::cout << "183" << std::endl;
    }

    cur_task = -1; 
    while (num_threads_done > 0) {
        std::cout << "188" << " " << num_threads_done << std::endl;
    }
    num_threads_done = 0; 

}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
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
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
