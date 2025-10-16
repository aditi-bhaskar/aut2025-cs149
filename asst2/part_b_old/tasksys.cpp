#include "tasksys.h"
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <cassert>

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

    // std::cout << "destructordestructordestructordestructordestructordestructordestructor " << std::endl;

    this->sync();

    // std::cout << "149" << std::endl;

    kill_threads = true; 
    cv.notify_all();
    while (num_threads_ready_to_die != n_threads - 1) {
        // std::cout << "154" << std::endl;
        cv.notify_all();
    }

    // std::cout << "158" << std::endl;

    for (int j=1; j< n_threads; j++) {
        workers[j].join();
    }
    // std::cout << "163" << std::endl;

    // todo: delete the structs allocated and stored in the list of readytorun at some point...
}


void TaskSystemParallelThreadPoolSleeping::sleepRunThread(int thread_id) {

    while(true) {
        std::unique_lock<std::mutex> lk(myMutex);

        while (!processing_tasks) {
            cv.wait(lk);
            // // std::cout << "thread awake! " << thread_id << std::endl;

            if (kill_threads) {
                lk.unlock();
                // num_threads_done++; 
                num_threads_ready_to_die++;
                return; 
            }
        }

        int my_task = cur_task_index; 

        if (my_task >= cur_num_total_tasks) {
            // std::cout << "182 " << cur_launch_id << std::endl;
            lk.unlock(); 
            num_threads_done += 1; 
            // std::cout << "185 " << num_threads_done << std::endl;
            if (num_threads_done == n_threads-1) {
                // std::cout << "setting processing_tasks to false " << std::endl;
                processing_tasks = false;
            }
            while (processing_tasks) {
            }
            // std::cout << "190 " << cur_launch_id << std::endl;
            this->rebalanceRunning();
            // std::cout << "192 " << cur_launch_id << " " << num_threads_done << std::endl;
            num_threads_done--;
            continue; 
        }

        cur_task_index += 1; 
        lk.unlock(); 

        cur_runnable->runTask(my_task, cur_num_total_tasks);
    }

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {

    // std::cout << "in run " << std::endl;

    this->sync();

    std::vector<TaskID> nodep = {};
    this->runAsyncWithDeps(runnable, num_total_tasks, nodep);

    this->sync();
}

// // this function is inside a mutex (is called from rebalance running)
// void TaskSystemParallelThreadPoolSleeping::grabNewLaunch(void) {

//     // std::cout << "grabNewLaunch" << std::endl;

//     // // std::cout << "grabNewLaunch " << std::endl;

//     if (ready_to_run.empty()) {
//         // std::cout << "ready to run empty" << std::endl;
//         processing_tasks = false; // there are currrently no tasks to process
//         // std::cout << "grabNewLaunch done" << std::endl;
//         return;
//     }

//     auto it = ready_to_run.begin();

//     cur_num_total_tasks = it->second->num_total_tasks;
//     cur_launch_id = it->second->id;
//     cur_runnable = it->second->task_runnable;

//     // std::cout << "starting " << cur_launch_id << std::endl;

//     cur_task_index = 0;
//     processing_tasks = true;

//     ready_to_run.erase(it);

//     // std::cout << "we've started up the task, so we can mark it as done " << cur_launch_id << std::endl;
//     done.push_back(cur_launch_id);

//     // std::cout << "grabNewLaunch done" << std::endl;

// }

// void TaskSystemParallelThreadPoolSleeping::rebalanceRunningOld(void) {

//     // std::cout << "rebalanceRunning" << std::endl;

//     // std::cout << "in rebalance running" << std::endl;

//     myMutex.lock(); // make sure there is no contention in grabbing the next launch

//     std::cout << "rebalanceRunning mutex" << std::endl;

//     // // std::cout << "rebalanceRunning " << std::endl;
//     std::vector<TaskID> entries_to_erase{};

//     // // std::cout << "len(launches_with_dep) " << launches_with_dep.size() << std::endl;

//     for(const auto& pair : launches_with_dep) {
//         // std::cout << pair.first << pair.second << std::endl;
//         bool dep_ok_to_run = true;
//         // // std::cout << "pair.first " << pair.first << std::endl;
//         for(const auto& dep : pair.second->dependencies) {
//             // std::cout << "280" << std::endl;
//             auto dep_in_done = std::find(done.begin(), done.end(), dep);
//             if(dep_in_done == done.end()) {
//                 dep_ok_to_run = false;
//             }
//         }
//         if (dep_ok_to_run) {
//             // std::cout << "286" << std::endl;
//             // // std::cout << "another launch ready to run! " << pair.first << std::endl;
//             ready_to_run[pair.first] = pair.second; // add entry into ready list
//             entries_to_erase.push_back(pair.first);

//             // std::cout << "rebalance running " << pair.second << " " << pair.second->id << std::endl;
//         }
//     }

//     // erase launches that we have added to 'ready_to_run'

//     std::cout << "296" << std::endl;

//     while(!entries_to_erase.empty()) {
//         // std::cout << "279" << std::endl;
//         // // std::cout << "erasing entry " << entries_to_erase.size() << std::endl;
//         // if (launches_with_dep.find(entries_to_erase.back()) == launches_with_dep.end()) {
//         //     // std::cout << "entry not found, segfault cause found!!" << std::endl;
//         // } 
//         launches_with_dep.erase(entries_to_erase.back()); // remove entry from launches list
//         entries_to_erase.pop_back();
//     }

//     // std::cout << "processing_tasks" << processing_tasks << std::endl;

//     // myMutex.lock(); // make sure there is no contention in grabbing the next launch
//     if (processing_tasks == false) {
//         // std::cout << "grab new launch from rebalance" << std::endl;
//         this->grabNewLaunch();
//         cv.notify_all();
//     }

//     std::cout << "rebalanceRunning done" << std::endl;

//     myMutex.unlock();

    
// }

void TaskSystemParallelThreadPoolSleeping::rebalanceRunning(void) {

    myMutex.lock();

    // std::cout << "starting rebalance" << std::endl;

    for(auto it = launches_with_dep.begin(); it != launches_with_dep.end(); it++) {
        // std::cout << it->first << std::endl;
        assert(it->second != nullptr);
        bool missed_dependency = false; 
        for(long unsigned int i = 0; i < it->second->dependencies.size(); i++) {
            if (std::find(done.begin(), done.end(), it->second->dependencies[i]) == done.end()) {
                missed_dependency = true; 
            }
        }
        if(!missed_dependency) {
            ready_to_run[it->first] = it->second; 
        }
    }

    // std::cout << "344" << std::endl;

    for(auto it = ready_to_run.begin(); it != ready_to_run.end(); it++) {
        if(launches_with_dep.find(it->first) != launches_with_dep.end()) {
            launches_with_dep.erase(it->first); 
        }
    }

    if(!processing_tasks && !ready_to_run.empty()) {

        auto it = ready_to_run.begin(); 
        auto [key, val] = *it; 
        ready_to_run.erase(it); 

        cur_task_index = 0; 
        cur_launch_id = key; 
        cur_runnable = val->task_runnable; 
        cur_num_total_tasks = val->num_total_tasks; 

        done.push_back(cur_launch_id); 
        processing_tasks = true; 
        cv.notify_all();
    }

    // std::cout << "ending rebalance" << std::endl;

    myMutex.unlock();

}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int n_total_tasks,
                                                    const std::vector<TaskID>& deps) {

    // // std::cout << "266 " << n_total_tasks << std::endl;

    // add the task and dependencies to the launches_with_dep map
    myMutex.lock();
    int launch_id = max_launch_id++;
    // std::cout << "LAUNCH ID " << launch_id << std::endl;
    

    LaunchInfo *launch_info = new LaunchInfo(launch_id, n_total_tasks, deps, runnable);
    // // std::cout << "new launch info " << *launch_info << " " << launch_id << // std::cout; 
    launches_with_dep[launch_id] = launch_info;

    myMutex.unlock();

    this->rebalanceRunning();
    this->sync();

    return (TaskID)launch_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    // // std::cout << "in sync " << std::endl;
    // TODO use a CV for this instead of spinning!
    while(!launches_with_dep.empty() || !ready_to_run.empty() || processing_tasks) {
        // // std::cout << "launches_with_dep " << launches_with_dep.size() << std::endl;
        // // std::cout << "ready_to_run " << ready_to_run.size() << std::endl;
    }

    return;
}


// todo:
// make sure we mark tasks as DONE
// make sure all accesses to ready_to_run are mutex blocked

