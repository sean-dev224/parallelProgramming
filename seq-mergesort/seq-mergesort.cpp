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

void divide_vector(std::vector<int>& vect, std::vector<int>& half1, std::vector<int>& half2) {
    for(int i = 0; i < vect.size() / 2; i++) {
        half1.push_back(vect[i]);
    }

    for(int i = vect.size() / 2; i < vect.size(); i++) {
        half2.push_back(vect[i]);
    }
}

std::vector<int> merge_vectors(std::vector<int>& half1, std::vector<int>& half2) {
    int half1_index = 0, half2_index = 0;
    std::vector<int> result;

    while(half1_index < half1.size() || half2_index < half2.size()) {
        //case for when one array is fully merged
        if(half1_index >= half1.size()) {
            result.push_back(half2[half2_index]);
            half2_index++;
            continue;
        }
        if(half2_index >= half2.size() ) {
            result.push_back(half1[half1_index]);
            half1_index++;
            continue;
        }

        //when both arrays still have elements
        if(half1[half1_index] < half2[half2_index]) {
            result.push_back(half1[half1_index]);
            half1_index++;
            continue;
        } else {
            result.push_back(half2[half2_index]);
            half2_index++;
            continue;
        }
    }

    return result;
}

std::vector<int> merge_sort(std::vector<int> vect) {
    int n = vect.size();

    //trigger end of recursive divide
    if(n == 1) {
        return vect;
    }

    //divide vector and recurse
    std::vector<int> half1;
    std::vector<int> half2;

    divide_vector(vect, half1, half2);

    half1 = merge_sort(half1);
    half2 = merge_sort(half2);

    //merge sorted halves back together
    return merge_vectors(half1, half2);
}

double test_merge(int n_magnitude) {
    namespace chrn = std::chrono;

    std::vector<int> data;
    generate_data(data, n_magnitude);

    auto start = chrn::high_resolution_clock::now();
    data = merge_sort(data);
    auto end = chrn::high_resolution_clock::now();
    auto elapsed_us = chrn::duration_cast<chrn::microseconds>(end - start).count();
    double elapsed_ms = elapsed_us / 1000.0;


    return elapsed_ms;
}

int main(int argc, char* argv[]) {
    std::ofstream csvFile("execution_times.csv");
    csvFile<<"n_magnitude,execution_time_ms\n";
    
    for(int i = 1; i <= 9; i++) {
        double elapsed_ms = test_merge(i);

        std::cout<<"n = 10^" << i << ", execution time: " << elapsed_ms <<" ms\n";
        csvFile<<i<<","<<elapsed_ms<<"\n";

    }

    csvFile.close();

    return 0;
}


