#include "tasksys.h"
#include <algorithm>
#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <condition_variable>

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
    // n_threads = num_threads+1; 
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

        if (kill_threads) {
            num_threads_done++; 
            return; 
        }
    
        if (cur_task == -1) {
            continue; 
        }


        myMutex.lock(); 
        int my_task = cur_task; 

        if (my_task >= num_total_tasks) {
            myMutex.unlock(); 
            num_threads_done += 1; 
            while (cur_task.load() != -1) {

            }
            num_threads_done--; 
            continue; 
        }

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
    // n_threads = num_threads + 1; // +1 for the main thread too
    workers = new std::thread[n_threads];

    for (int j = 1; j < n_threads; j++) {
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSpinning::spinRunThread, this, j);
    }
    
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {

    kill_threads = true; 
    while (num_threads_done != n_threads - 1) {
    }

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

    myMutex.lock();
    cur_task = 0; 
    myMutex.unlock();
    

    while(num_threads_done < n_threads - 1) {
    }

    myMutex.lock();
    cur_task = -1; 
    myMutex.unlock();

    while (num_threads_done > 0) {
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

    n_threads = num_threads; 
    workers = new std::thread[n_threads];

    for (int j = 1; j < n_threads; j++) {
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSleeping::sleepRunThread, this, j);
    }
    
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {


    kill_threads = true; 
    cv.notify_all();
    while (num_threads_done != n_threads - 1) {
        cv.notify_all();
    }

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }
}

void TaskSystemParallelThreadPoolSleeping::sleepRunThread(int thread_id) {

    while(true) {

        // myMutex.lock(); 
        std::unique_lock<std::mutex> lk(myMutex);

        while (cur_task == -1) {

            cv.wait(lk); 
            if (kill_threads) {
                lk.unlock();
                num_threads_done++; 
                return; 
            }
        }

        int my_task = cur_task; 


        // OLD FUNCTIONAL VERSION

        if (my_task >= num_total_tasks) {
            lk.unlock(); 
            num_threads_done += 1; 
            // std::cout << num_threads_done << std::endl;
            while (cur_task.load() != -1) {

            }
            num_threads_done--; 
            continue; 
        }

        // NEW VERSION WITH CV, worse performance
        // if (my_task >= num_total_tasks) {
        //     lk.unlock(); 

        //     std::unique_lock<std::mutex> ctlk(cur_task_mutex);
        //     // std::cout << "waiting" << std::endl;

        //     num_threads_done += 1; 
        //     cur_task_neg1.wait(ctlk);
        //     num_threads_done--; 

        //     // std::cout << "finished waiting" << std::endl;
        //     ctlk.unlock();

        //     continue; 
        // }


        cur_task += 1; 
        lk.unlock(); 

        run_function->runTask(my_task, num_total_tasks);
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int n_total_tasks) {

    run_function = runnable; 

    num_threads_done = 0; 
    num_total_tasks = n_total_tasks; 

    myMutex.lock();
    cur_task = 0; 
    cv.notify_all(); 
    myMutex.unlock();
    
    while(num_threads_done < n_threads - 1) {} // spin until all threads reach here

    // set the task to -1, and then notify all threads to continue
    myMutex.lock();
    cur_task = -1; 
    myMutex.unlock();
    // cur_task_neg1.notify_all(); //  for version with cv, 

    // wait for all threads to finish execution (barrier)
    while (num_threads_done > 0) {}

    num_threads_done = 0; 

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
