#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <chrono>

void generate_data(std::vector<int>& result, int n_magnitude) {
    for(int i = 0; i < pow(10, n_magnitude); i++) {
        result.push_back(rand());
    }
}

void print_vector(const std::vector<int>& vector_to_print) {
    for(int n: vector_to_print) {
        std::cout << n << " ";
    }
    std::cout << "\n";
}

void verify_sorted(const std::vector<int>& arr) {
    for(int i = 1; i < arr.size(); i++) {
        if(arr[i-1] > arr[i]) {
            std::cout<<"NOT SORTED\n";
            return;
        }
    }
    std::cout<<"Sorted successfully\n";
}

void merge_vectors_inplace(std::vector<int>& arr, int left, int mid, int right) {
    //half1 is from [left, mid], half2 is from [mid+1, right]
    int half1_n = mid - left + 1;
    int half2_n = right - mid;

    //copy values to temporary vectors
    std::vector<int> half1;
    half1.reserve(half1_n);
    for(int i = left; i <= mid; i++) {
        half1.push_back(arr[i]);
    }

    std::vector<int> half2;
    half2.reserve(half2_n);
    for(int i = mid+1; i <= right; i++) {
        half2.push_back(arr[i]);
    }

    //merge temp vectors back to original vector
    int half1_i = 0; int half2_i = 0; int arr_i = left;

    while(half1_i < half1.size() && half2_i < half2.size()) {
        if(half1[half1_i] <= half2[half2_i]) {
            arr[arr_i] = half1[half1_i];
            half1_i++;
            arr_i++;
        } else {
            arr[arr_i] = half2[half2_i];
            half2_i++;
            arr_i++;
        }
    }
    //add remaining elements once one array has been traversed
    while(half1_i < half1.size()) {
        arr[arr_i] = half1[half1_i];
            half1_i++;
            arr_i++;
    }
    while(half2_i < half2.size()) {
        arr[arr_i] = half2[half2_i];
            half2_i++;
            arr_i++;
    }
}

void merge_sort(std::vector<int>& arr, int left, int right) {

    //trigger end of recursive divide
    if(left >= right) {
        return;
    }

    int mid = left + (right - left) / 2;
    merge_sort(arr, left, mid);
    merge_sort(arr, mid+1, right);
    merge_vectors_inplace(arr, left, mid, right);
}

double test_merge(int n_magnitude) {
    namespace chrn = std::chrono;

    std::vector<int> data;
    generate_data(data, n_magnitude);

    auto start = chrn::high_resolution_clock::now();
    merge_sort(data, 0, data.size()-1);
    auto end = chrn::high_resolution_clock::now();
    auto elapsed_us = chrn::duration_cast<chrn::microseconds>(end - start).count();
    double elapsed_ms = elapsed_us / 1000.0;
    
    // print_vector(data);
    // verify_sorted(data);
    
    return elapsed_ms;
}

int main(int argc, char* argv[]) {
    std::ofstream csvFile("execution_times.csv");
    csvFile<<"n_magnitude,execution_time_ms\n";
    
    for(int i = 1; i <= 8; i++) {
        double elapsed_ms = test_merge(i);

        std::cout<<"n = 10^" << i << ", execution time: " << elapsed_ms <<" ms\n";
        csvFile<<i<<","<<elapsed_ms<<"\n";

    }

    csvFile.close();

    return 0;
}


