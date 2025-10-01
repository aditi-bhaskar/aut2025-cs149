#include <stdio.h>
#include <thread>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "CycleTimer.h"


#define MAX_THREADS 32

float worker_thread_durations[MAX_THREADS] = {0};

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


extern void mandelbrotThreadEqualizer(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int threadId, int numThreads,
    int maxIterations,
    int output[]);
//
// workerThreadStart --
//
// Thread entrypoint.
void workerThreadStart(WorkerArgs * const args) {

// void mandelbrotSerial(
//     float x0, float y0, float x1, float y1,
//     int width, int height,
//     int startRow, int totalRows,
//     int maxIterations,
//     int output[])
// {

    // timing
    float worker_thread_start_time = CycleTimer::currentSeconds();

    int startRow = args->threadId * ceil(1.0 * args->height / args->numThreads);
    int max_total_rows = (int) ceil(1.0 * args->height / args->numThreads);
    int remaining_rows = (int) args->height - startRow;
    int totalRows = std::min(max_total_rows, remaining_rows); // ceiling of float division

    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, args->width, args->height, 
                    startRow, totalRows, args->maxIterations, args->output);


    // mandelbrotThreadEqualizer(args->x0, args->y0, args->x1, args->y1, args->width, args->height, 
    //                 args->threadId, args->numThreads, args->maxIterations, args->output);

    float worker_thread_duration = CycleTimer::currentSeconds() - worker_thread_start_time;

    // timing
    worker_thread_durations[args->threadId] += worker_thread_duration;

}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    // static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
      
        // TODO FOR CS149 STUDENTS: You may or may not wish to modify
        // the per-thread arguments here.  The code below copies the
        // same arguments for each thread
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
      
        args[i].threadId = i;
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStart, &args[i]);
    }
    
    workerThreadStart(&args[0]);

    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();
    }

    for (int i=0; i<numThreads; i++) {
        std::cout << "worker_thread_duration: " << i << " : " << worker_thread_durations[i] << std::endl;
    }

}
