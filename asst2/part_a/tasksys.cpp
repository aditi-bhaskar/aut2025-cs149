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

        // std::cout << cur_task << std::endl;

        if (kill_threads) {
            // std::cout << "kill_threads" << " " << num_threads_done << " " << thread_id << std::endl;
            num_threads_done++; 
            return; 
        }
    
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
            // std::cout << num_threads_done << std::endl;
            while (cur_task.load() != -1) {

            }
            num_threads_done--; 
            continue; 
        }

        // std::cout << "135" << " " << my_task << " " << cur_task << std::endl;

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
        // std::cout << "starting thread " << j << std::endl;
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSpinning::spinRunThread, this, j);
    }
    
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {

    // std::cout << "destructor" << std::endl; 

    kill_threads = true; 
    while (num_threads_done != n_threads - 1) {
        // std::cout << "185" << num_threads_done << " " << n_threads << std::endl;
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

    // std::cout << "new run" << std::endl; 

    run_function = runnable; 

    num_threads_done = 0; 
    num_total_tasks = n_total_tasks; 

    myMutex.lock();
    cur_task = 0; 
    myMutex.unlock();
    

    while(num_threads_done < n_threads - 1) {
        // std::cout << "183" << std::endl;
    }

    myMutex.lock();
    cur_task = -1; 
    myMutex.unlock();

    while (num_threads_done > 0) {
        // std::cout << "188" << " " << num_threads_done << std::endl;
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


void TaskSystemParallelThreadPoolSleeping::sleepRunThread(int thread_id) {

    while(true) {

        // myMutex.lock(); 
        std::unique_lock<std::mutex> lk(myMutex);

        while (cur_task == -1) {
            // std::cout << "259 " << num_threads_done << std::endl;
            // if (kill_threads) {
            //     lk.unlock();
            //     std::cout << "263 " << num_threads_done << std::endl;
            //     num_threads_done++; 
            //     return; 
            // }
            cv.wait(lk); 
            // std::cout << "261" << std::endl;
            if (kill_threads) {
                lk.unlock();
                // std::cout << "263 " << num_threads_done << std::endl;
                num_threads_done++; 
                return; 
            }
        }

        // std::cout << "here" << std::endl;

        int my_task = cur_task; 

        // std::cout << "271 " << my_task << std::endl;

        if (my_task >= num_total_tasks) {
            lk.unlock(); 
            num_threads_done += 1; 
            // std::cout << num_threads_done << std::endl;
            while (cur_task.load() != -1) {

            }
            num_threads_done--; 
            continue; 
        }

        // if (my_task >= num_total_tasks) {
        //     lk.unlock(); 
        //     num_threads_done += 1; 

        //     std::unique_lock<std::mutex> lk2(myMutex2);

        //     // if (num_threads_done == n_threads - 2) {
        //     //     lk2.unlock(); 
        //     //     cv2.notify_all(); 
        //     //     std::cout << "notify" << std::endl;
        //     //     std::cout << num_threads_done << " " << n_threads << std::endl;
        //     // } else {
        //     //     std::cout << num_threads_done << " " << n_threads << std::endl;
        //     //     cv2.wait(lk2); 
        //     //     lk2.unlock(); 
        //     // }

        //     // std::cout << num_threads_done << " " << n_threads << std::endl;
        //     // cv2.wait(lk2); 
            
            
        //     // num_threads_done--; 
        //     // std::cout << "291 " << num_threads_done << " " << cur_task << std::endl;

        //     // lk2.unlock(); 

        //     continue; 
        // }

        cur_task += 1; 
        lk.unlock(); 

        run_function->runTask(my_task, num_total_tasks);
    }
}


TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
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
        workers[j] = std::thread(&TaskSystemParallelThreadPoolSleeping::sleepRunThread, this, j);
    }
    
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {

    // std::cout << "destructor" << std::endl;

    kill_threads = true; 
    cv.notify_all();
    // std::cout << "349 notify" << std::endl;
    while (num_threads_done != n_threads - 1) {
        cv.notify_all();
        // std::cout << "347 " << num_threads_done << " " << cur_task << std::endl;
    }

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int n_total_tasks) {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // std::cout << "new run" << std::endl; 

    run_function = runnable; 

    num_threads_done = 0; 
    num_total_tasks = n_total_tasks; 

    myMutex.lock();
    cur_task = 0; 
    cv.notify_all(); 
    myMutex.unlock();
    

    while(num_threads_done < n_threads - 1) {
    }
    // std::cout << "365 " << num_threads_done << std::endl;
    

    myMutex.lock();
    cur_task = -1; 
    myMutex.unlock();

    // cv2.notify_all();
    // std::cout << "cv2 notify" << std::endl;

    while (num_threads_done > 0) {
        // std::cout << "367 " << num_threads_done << std::endl;
    }
    num_threads_done = 0; 

    // std::cout << "exit run" << std::endl;

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
