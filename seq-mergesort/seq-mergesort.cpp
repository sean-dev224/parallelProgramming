#include <iostream>
#include <vector>
#include <cmath>
#include <string>

std::vector<int> generate_data(int exponent_size) {
    std::vector<int> result;

    for(int i = 0; i < pow(10, exponent_size); i++) {
        result.push_back(rand());
    }

    return result;
}

int main(int argc, char* argv[]) {

    auto data = generate_data(2);

    //std::cout<<string::to_string(data)<<"\n";


    return 0;
}


