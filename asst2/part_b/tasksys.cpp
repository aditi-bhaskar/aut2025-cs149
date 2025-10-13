#include "tasksys.h"
#include <iostream>
#include <algorithm>
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

    std::cout << "destructor " << std::endl;

    this->sync();

    kill_threads = true; 
    cv.notify_all();
    while (num_threads_done != n_threads - 1) {
        cv.notify_all();
    }

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }

    // todo should delete the structs allocated and stored in the list of readytorun at some point...

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

            if (kill_threads) {
                lk.unlock();
                num_threads_done++; 
                return; 
            }
        }

        int my_task = cur_task_index; 

        if (my_task >= cur_num_total_tasks) {
            lk.unlock(); 
            num_threads_done += 1; 
            if (num_threads_done == n_threads-1) {
                processing_tasks = false;
            }
            while (processing_tasks) {
            }
            this->rebalanceRunning();
            num_threads_done--;
            continue; 
        }

        cur_task_index += 1; 
        lk.unlock(); 

        cur_runnable->runTask(my_task, cur_num_total_tasks);
    }

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {

    std::cout << "in run " << std::endl;

    this->sync();

    std::vector<TaskID> nodep = {};
    this->runAsyncWithDeps(runnable, num_total_tasks, nodep);

    this->sync();
}

// this function is inside a mutex (is called from rebalance running)
void TaskSystemParallelThreadPoolSleeping::grabNewLaunch(void) {

    // std::cout << "grabNewLaunch " << std::endl;

    if (ready_to_run.empty()) {
        processing_tasks = false; // there are currrently no tasks to process
    }

    auto it = ready_to_run.begin();

    cur_num_total_tasks = it->second->num_total_tasks;
    cur_task_id = it->second->id;
    cur_runnable = it->second->task_runnable;

    cur_task_index = 0;
    processing_tasks = true;

    ready_to_run.erase(it);
}

void TaskSystemParallelThreadPoolSleeping::rebalanceRunning(void) {

    myMutex.lock(); // make sure there is no contention in grabbing the next launch

    // std::cout << "rebalanceRunning " << std::endl;
    std::vector<TaskID> entries_to_erase{};
    for(const auto& pair : launches_with_dep) {
        bool dep_ok_to_run = true;
        for(const auto& dep : pair.second->dependencies) {
            auto dep_in_done = std::find(done.begin(), done.end(), dep);
            if(dep_in_done == done.end()) {
                dep_ok_to_run = false;
            }
        }
        if (dep_ok_to_run) {
            std::cout << "another launch ready to run! " << pair.first << std::endl;
            ready_to_run[pair.first] = pair.second; // add entry into ready list
            entries_to_erase.push_back(pair.first);
        }
    }

    // erase launches that we have added to 'ready_to_run'
    while(!entries_to_erase.empty()) {
        std::cout << "erasing entry " << entries_to_erase.size() << std::endl;
        launches_with_dep.erase(entries_to_erase.back()); // remove entry from launches list
        entries_to_erase.pop_back();
    }

    // myMutex.lock(); // make sure there is no contention in grabbing the next launch
    if (processing_tasks == false) {
        this->grabNewLaunch();
        cv.notify_all();
    }

    myMutex.unlock();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int n_total_tasks,
                                                    const std::vector<TaskID>& deps) {

    std::cout << "266 " << n_total_tasks << std::endl;

    // add the task and dependencies to the launches_with_deps map
    myMutex.lock();
    int launch_id = max_launch_id++;
    myMutex.unlock();

    LaunchInfo *launch_info = new LaunchInfo(launch_id, n_total_tasks, deps, runnable);
    launches_with_dep[launch_id] = launch_info;

    this->rebalanceRunning();

    return (TaskID)launch_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    std::cout << "in sync " << std::endl;
    // TODO use a CV for this instead of spinning!
    while(!launches_with_dep.empty() || !ready_to_run.empty()) {
        std::cout << "launches_with_dep " << launches_with_dep.size() << std::endl;
        std::cout << "ready_to_run " << ready_to_run.size() << std::endl;
        // sleep(1);
    }

    return;
}
