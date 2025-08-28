#include <iostream>
#include <vector>
#include <cmath>
#include <string>

void generate_data(std::vector<int>& result, int exponent_size) {
    for(int i = 0; i < pow(10, exponent_size); i++) {
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
    std::vector<int> result(half1.size() + half2.size());

    while(!half1.empty() && !half2.empty()) {
        if(half1.empty()) {
            result.push_back()
        }
    }
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

int main(int argc, char* argv[]) {
    std::vector<int> result;

    generate_data(result, 0);
    print_vector(result);

    std::vector<int> half1;
    std::vector<int> half2;

    divide_vector(result, half1, half2);

    print_vector(half1);
    print_vector(half2);

    return 0;
}


