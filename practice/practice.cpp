#include <iostream>
#include <vector>
#include <list>

int main() {

    int i = 0;

    std::cout<<i<<"\n";

    while (i < 15) {
        std::cout<<"i: "<<i++<<"\n";
    }

    std::vector<int> v = {1, 2, 3};

    for(int i = 0; i < v.size(); i++){
        std::cout<<"vector value " <<i<<": "<<v[i]<<"\n";
    }
}