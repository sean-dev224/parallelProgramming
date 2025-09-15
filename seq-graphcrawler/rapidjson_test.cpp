#include <string>
#include "rapidjson/document.h"
#include <iostream>

int main () {
    std::string json = "{\
        \"hello\": \"world\"\
    }";

    using namespace rapidjson;
    Document document;
    //convert our c++ string to a c string to pass to rapidjson
    document.Parse(json.c_str());

    std::string value = document["hello"].GetString();

    std::cout<<value<<"\n";


    return 0;
}