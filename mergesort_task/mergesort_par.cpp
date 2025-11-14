#include <omp.h>
#include "../tooling/omp_tasking.hpp"
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <vector>


#define DEBUG 0

static int SEQ_CUTOFF = 1000;

void generateMergeSortData (std::vector<int>& arr, size_t n) {
  for (size_t  i=0; i< n; ++i) {
    arr[i] = rand();
  }
}
  
void checkMergeSortResult (std::vector<int>& arr, size_t n) {
  bool ok = true;
  for (size_t  i=1; i<n; ++i)
    if (arr[i]< arr[i-1])
      ok = false;
  if(!ok)
    std::cerr<<"notok"<<std::endl;
}


void merge(int * arr, size_t  l, size_t  mid, size_t r, int* temp) {
  
#if DEBUG
  std::cout<<"Splits: "<<l<<" "<<mid<<" "<<r<<std::endl;
#endif

  // short circuits
  if (l == r) return;
  if (r-l == 1) { //case where l and r are both one number
    if (arr[l] > arr[r]) {
      int swap = arr[l];
      arr[l] = arr[r];
      arr[r] = swap;
    }
    return;
  }

  size_t i, j, k;
  size_t n = mid - l;
  
  // init temp arrays. copy l into temp at the correct indexes
  for (i=l; i<mid; ++i)
    temp[i] = arr[i];

  i = l;    // temp left half
  j = mid;  // right half
  k = l;    // write to 

  // merge
  while (i<mid && j<=r) {
     if (temp[i] <= arr[j] ) {
       arr[k++] = temp[i++];
     } else {
       arr[k++] = arr[j++];
     }
  }

  // exhaust temp 
  while (i<mid) {
    arr[k++] = temp[i++];
  }

}

void seq_mergesort(int * arr, size_t l, size_t r, int* temp) {
  if (l < r) {
    size_t mid = (l+r)/2;
    seq_mergesort(arr, l, mid, temp);
    seq_mergesort(arr, mid+1, r, temp);
    merge(arr, l, mid+1, r, temp);
  }
}

void mergesort(int * arr, size_t l, size_t r, int* temp) {
  if (l >= r) {
    //do nothing
  } else if (r-l < SEQ_CUTOFF) {
    size_t mid = (l+r)/2;
    seq_mergesort(arr, l, mid, temp);
    seq_mergesort(arr, mid+1, r, temp);
    merge(arr, l, mid+1, r, temp);
  } else {
    size_t mid = (l+r)/2;

    tasking::taskstart([&](){
      mergesort(arr, l, mid, temp);
    });

    tasking::taskstart([&](){
      mergesort(arr, mid+1, r, temp);
    });

    tasking::taskwait();
    
    merge(arr, l, mid+1, r, temp);
  }
}

void task_test() {
  std::function<void()> waste_time = []() {
    std::cout<<"Task starting\n";
    sleep(2);
    std::cout<<"Task finished\n";
  };

  tasking::doinparallel([&waste_time](){
    std::cout<<"Starting Parallel Tasks\n";
    
    tasking::taskstart([&waste_time](){waste_time();});
    tasking::taskstart([&waste_time](){waste_time();});

    tasking::taskwait();
  }, 8);
}


int main (int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }
  
  // command line parameter
  size_t n = atol(argv[1]);
  int nbthreads = atol(argv[2]);

  // get arr data
  std::vector<int> arr (n);
  generateMergeSortData (arr, n);

#if DEBUG
  std::cout<<"Starting Array:\n";
  for (size_t i=0; i<n; ++i) 
    std::cout<<arr[i]<<" ";
  std::cout<<std::endl;
#endif

  // begin timing
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  
  std::vector<int> temp (n);
  
  
  // sort
  tasking::doinparallel([&](){
    mergesort(&(arr[0]), 0, n-1, &(temp[0]));
  }, nbthreads);
  

  // end timing
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elpased_seconds = end-start;

  // display time to cerr
  std::cerr<<"Execution Time: "<<elpased_seconds.count()<<"s"<<std::endl;
  checkMergeSortResult (arr, n);

#if DEBUG
  std::cout<<"Sorted Array: \n";
  for (size_t i=0; i<n; ++i) 
    std::cout<<arr[i]<<" ";
  std::cout<<std::endl;
#endif

  return 0;
}
