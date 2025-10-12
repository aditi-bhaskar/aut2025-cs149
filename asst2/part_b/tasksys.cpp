#include "tasksys.h"
#include <algorithm>

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
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    // kill_threads = true; 
    // cv.notify_all();
    // // std::cout << "349 notify" << std::endl;
    // while (num_threads_done != n_threads - 1) {
    //     cv.notify_all();
    //     // std::cout << "347 " << num_threads_done << " " << cur_task << std::endl;
    // }

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }

}


void TaskSystemParallelThreadPoolSleeping::sleepRunThread(int thread_id) {


    // create the worker helper function here

    // process curtask (like before)

    // when we finish all the indices in a range, 
    // pop off the task from ready_to_run, put it into done
    // call rebalance ready_to_run list

    while(true) {
        std::unique_lock<std::mutex> lk(myMutex);

        while (!processing_tasks) {
            cv.wait(lk); 

            // TODO implement thread killing
            // if (kill_threads) {
            //     lk.unlock();
            //     num_threads_done++; 
            //     return; 
            // }
        }

        int my_task = cur_task_index; 

        if (my_task >= cur_num_total_tasks) {
            lk.unlock(); 
            num_threads_done += 1; 
            // TODO : in the main orchestrating thread, make sure to call rebalance and grabnewlaunch before setting processing_tasks=false
            while (processing_tasks) {
            }
            num_threads_done--; 
            continue; 
        }

        cur_task_index += 1; 
        lk.unlock(); 

        cur_runnable->runTask(my_task, cur_num_total_tasks);
    }

}


    

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {

    std::vector<TaskID> nodep = {};
    this->runAsyncWithDeps(runnable, num_total_tasks, nodep);
    this->sync();
}



void TaskSystemParallelThreadPoolSleeping::grabNewLaunch(void) {

    if (ready_to_run.empty()) {
        processing_tasks = false; // there are currrently no tasks to process
    }

    // put all of this in a mutex
    auto it = ready_to_run.begin();

    cur_num_total_tasks = it->second->num_total_tasks;
    cur_task_id = it->second->id;
    cur_runnable = it->second->task_runnable;

    cur_task_index = 0;
    processing_tasks = true;

    ready_to_run.erase(it);
    // end mutex
}

void TaskSystemParallelThreadPoolSleeping::rebalanceRunning(void) {

    for(const auto& pair : launches_with_dep) {
        bool dep_ok_to_run = true;
        for(const auto& dep : pair.second->dependencies) {
            auto dep_in_done = std::find(done.begin(), done.end(), dep);
            if(dep_in_done == done.end()) {
                dep_ok_to_run = false;
            }
        }
        if (dep_ok_to_run) {
           ready_to_run[pair.first] = pair.second;
        }
    }

    if (processing_tasks == false) {
        this->grabNewLaunch();
        cv.notify_all();
    }

    // re-check the launches_with_deps list and move unblocked launches into the ready_to_run vector
    // adding something to the ready_to_run vector also includes updating the vectors of the num total tasks and stuff
}


TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int n_total_tasks,
                                                    const std::vector<TaskID>& deps) {

    // add the task and dependencies to the launches_with_deps map
    int launch_id = max_launch_id++; // TODO put a mutex around this

    LaunchInfo *launch_info = new LaunchInfo(launch_id, n_total_tasks, deps, runnable);
    launches_with_dep[max_launch_id] = launch_info;

    this->rebalanceRunning();

    return (TaskID)launch_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    // TODO use a CV for this
    while(!(launches_with_dep.empty() && ready_to_run.empty())) {}

    return;
}
