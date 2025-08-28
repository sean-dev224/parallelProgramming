#include <iostream>
#include <string>
#include <vector>
#include <list>

class Person {
    private:
    std::string name;
    int age;
    void (*say_a_thing)();
    

    public:
    Person(std::string n, int a, void (*fptr)()) {
        name = n;
        age = a;
        say_a_thing = fptr;
    }

    std::string print_info() const {
        say_a_thing();
        return name + " " + std::to_string(age);
    }



};

void say_hello() {
    std::cout<<"Hello!\n";
}

int main() {

    Person p("Jeff", 32, say_hello);

    std::cout << p.print_info() << std::endl;

}