#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include "CycleTimer.h"
#include <iostream>

using namespace std;

typedef struct {
  // Control work assignments
  int start, end;

  // Shared by all functions
  double *data;
  double *clusterCentroids;
  int *clusterAssignments;
  double *currCost;
  int M, N, K;
} WorkerArgs;


/**
 * Checks if the algorithm has converged.
 * 
 * @param prevCost Pointer to the K dimensional array containing cluster costs 
 *    from the previous iteration.
 * @param currCost Pointer to the K dimensional array containing cluster costs 
 *    from the current iteration.
 * @param epsilon Predefined hyperparameter which is used to determine when
 *    the algorithm has converged.
 * @param K The number of clusters.
 * 
 * NOTE: DO NOT MODIFY THIS FUNCTION!!!
 */
static bool stoppingConditionMet(double *prevCost, double *currCost,
                                 double epsilon, int K) {
  for (int k = 0; k < K; k++) {
    if (abs(prevCost[k] - currCost[k]) > epsilon)
      return false;
  }
  return true;
}

/**
 * Computes L2 distance between two points of dimension nDim.
 * 
 * @param x Pointer to the beginning of the array representing the first
 *     data point.
 * @param y Poitner to the beginning of the array representing the second
 *     data point.
 * @param nDim The dimensionality (number of elements) in each data point
 *     (must be the same for x and y).
 */
double dist(double *x, double *y, int nDim) {
  double accum = 0.0;
  for (int i = 0; i < nDim; i++) {
    accum += pow((x[i] - y[i]), 2);
  }
  return sqrt(accum);
}

#define MAX_THREADS 32

typedef struct {
  double *data;
  int *clusterAssignments;
  double *minDist;
  int localM;
  double *clusterCentroids;
  int N;
  // int k;
  int totalk; 

  int threadId;
  int numThreads;
} HelperArgs;

// helper function for each thread
// void workerThread(HelperArgs *const thread_args) {

//     for (int m = 0; m < thread_args->localM; m++) {
//       double d = dist(&thread_args->data[m * thread_args->N],
//                       &thread_args->clusterCentroids[thread_args->k * thread_args->N], thread_args->N);
//       if (d < thread_args->minDist[m]) {
//         thread_args->minDist[m] = d;
//         thread_args->clusterAssignments[m] = thread_args->k;
//       }
//     }

// }


void workerThread(HelperArgs *const thread_args) {

  for(int k = 0; k < thread_args->totalk; k++) {

    for (int m = 0; m < thread_args->localM; m++) {
      double d = dist(&thread_args->data[m * thread_args->N],
                      &thread_args->clusterCentroids[k * thread_args->N], thread_args->N);
      if (d < thread_args->minDist[m]) {
        thread_args->minDist[m] = d;
        thread_args->clusterAssignments[m] = k;
      }
    }

  }

}


/**
 * Assigns each data point to its "closest" cluster centroid.
 */
void computeAssignments(WorkerArgs *const args) {
  double *minDist = new double[args->M];
  
  // Initialize arrays
  for (int m =0; m < args->M; m++) {
    minDist[m] = 1e30;
    args->clusterAssignments[m] = -1;
  }

  // float dist_time = 0;
  // int num_iterations = 0;
  // float sum_time = 0;
  // Assign datapoints to closest centroids
  // for (int k = args->start; k < args->end; k++) {

  //   std::thread workers[MAX_THREADS];
  //   HelperArgs helper_args[MAX_THREADS];
  //   int numThreads = 16; // TODO !

  //   // set up the threads
  //   for (int i=0; i<numThreads; i++) {

  //         helper_args[i].data = &args->data[args->M  * i / numThreads];
  //         helper_args[i].clusterAssignments = &args->clusterAssignments[args->M * i / numThreads];
  //         helper_args[i].minDist = &minDist[args->M * i / numThreads];
  //         helper_args[i].localM = args->M / numThreads; 
  //         helper_args[i].clusterCentroids = args->clusterCentroids;

  //         helper_args[i].N = args->N;
  //         helper_args[i].k = k;

  //         helper_args[i].threadId = i;
  //         helper_args[i].numThreads = numThreads;

  //   }



    std::thread workers[MAX_THREADS];
    HelperArgs helper_args[MAX_THREADS];
    int numThreads = 16; // TODO !

    // set up the threads
    for (int i=0; i<numThreads; i++) {

          helper_args[i].data = &args->data[args->M  * i / numThreads];
          helper_args[i].clusterAssignments = &args->clusterAssignments[args->M * i / numThreads];
          helper_args[i].minDist = &minDist[args->M * i / numThreads];
          helper_args[i].localM = args->M / numThreads; 
          helper_args[i].clusterCentroids = args->clusterCentroids;

          helper_args[i].N = args->N;
          // helper_args[i].k = k;
          helper_args[i].totalk = args->end;

          helper_args[i].threadId = i;
          helper_args[i].numThreads = numThreads;

    }

  

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThread, &helper_args[i]);
    }

    workerThread(&helper_args[0]);

    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();
    }


  free(minDist);
}



/**
 * Given the cluster assignments, computes the new centroid locations for
 * each cluster.
 */
void computeCentroids(WorkerArgs *const args) {
  int *counts = new int[args->K];

  // Zero things out
  for (int k = 0; k < args->K; k++) {
    counts[k] = 0;
    for (int n = 0; n < args->N; n++) {
      args->clusterCentroids[k * args->N + n] = 0.0;
    }
  }


  // Sum up contributions from assigned examples
  for (int m = 0; m < args->M; m++) {
    int k = args->clusterAssignments[m];
    for (int n = 0; n < args->N; n++) {
      args->clusterCentroids[k * args->N + n] +=
          args->data[m * args->N + n];
    }
    counts[k]++;
  }

  // Compute means
  for (int k = 0; k < args->K; k++) {
    counts[k] = max(counts[k], 1); // prevent divide by 0
    for (int n = 0; n < args->N; n++) {
      args->clusterCentroids[k * args->N + n] /= counts[k];
    }
  }

  free(counts);
}

/**
 * Computes the per-cluster cost. Used to check if the algorithm has converged.
 */
void computeCost(WorkerArgs *const args) {
  double *accum = new double[args->K];

  // Zero things out
  for (int k = 0; k < args->K; k++) {
    accum[k] = 0.0;
  }

  // Sum cost for all data points assigned to centroid
  for (int m = 0; m < args->M; m++) {
    int k = args->clusterAssignments[m];
    accum[k] += dist(&args->data[m * args->N],
                     &args->clusterCentroids[k * args->N], args->N);
  }

  // Update costs
  for (int k = args->start; k < args->end; k++) {
    args->currCost[k] = accum[k];
  }

  free(accum);
}

/**
 * Computes the K-Means algorithm, using std::thread to parallelize the work.
 *
 * @param data Pointer to an array of length M*N representing the M different N 
 *     dimensional data points clustered. The data is layed out in a "data point
 *     major" format, so that data[i*N] is the start of the i'th data point in 
 *     the array. The N values of the i'th datapoint are the N values in the 
 *     range data[i*N] to data[(i+1) * N].
 * @param clusterCentroids Pointer to an array of length K*N representing the K 
 *     different N dimensional cluster centroids. The data is laid out in
 *     the same way as explained above for data.
 * @param clusterAssignments Pointer to an array of length M representing the
 *     cluster assignments of each data point, where clusterAssignments[i] = j
 *     indicates that data point i is closest to cluster centroid j.
 * @param M The number of data points to cluster.
 * @param N The dimensionality of the data points.
 * @param K The number of cluster centroids.
 * @param epsilon The algorithm is said to have converged when
 *     |currCost[i] - prevCost[i]| < epsilon for all i where i = 0, 1, ..., K-1
 */
void kMeansThread(double *data, double *clusterCentroids, int *clusterAssignments,
               int M, int N, int K, double epsilon) {

  // Used to track convergence
  double *prevCost = new double[K];
  double *currCost = new double[K];

  // The WorkerArgs array is used to pass inputs to and return output from
  // functions.
  WorkerArgs args;
  args.data = data;
  args.clusterCentroids = clusterCentroids;
  args.clusterAssignments = clusterAssignments;
  args.currCost = currCost;
  args.M = M;
  args.N = N;
  args.K = K;

  // Initialize arrays to track cost
  for (int k = 0; k < K; k++) {
    prevCost[k] = 1e30;
    currCost[k] = 0.0;
  }

  float assign_comp = 0;
  float centroids_comp = 0;
  float cost_comp = 0;
  int num_loops = 0;

  /* Main K-Means Algorithm Loop */
  int iter = 0;
  while (!stoppingConditionMet(prevCost, currCost, epsilon, K)) {
    // Update cost arrays (for checking convergence criteria)
    for (int k = 0; k < K; k++) {
      prevCost[k] = currCost[k];
    }
    num_loops++;

    // Setup args struct
    args.start = 0;
    args.end = K;

    float start_time = CycleTimer::currentSeconds();
    computeAssignments(&args);
    assign_comp += CycleTimer::currentSeconds() - start_time;

    start_time = CycleTimer::currentSeconds();
    computeCentroids(&args);
    centroids_comp += CycleTimer::currentSeconds() - start_time;

    start_time = CycleTimer::currentSeconds();
    computeCost(&args);
    cost_comp += CycleTimer::currentSeconds() - start_time;

    iter++;
  }

  assign_comp /= num_loops;
  centroids_comp /= num_loops;
  cost_comp /= num_loops;
  
  std::cout << "assign_comp: " << assign_comp << std::endl;
  std::cout << "centroids_comp: " << centroids_comp << std::endl;
  std::cout << "cost_comp: " << cost_comp << std::endl;

  free(currCost);
  free(prevCost);
}
