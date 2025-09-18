#include <curl/curl.h>
#include "rapidjson/document.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <chrono>

const std::string ENDPOINT = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// void replace_spaces(std::string& s, std::string replacement) {
//     for(size_t i = 0; i < s.length(); ++i) {
//         if(s[i] == ' ') {
//             s.replace(i, 1, replacement);
//         }
//     }
// }

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //cast the void pointer from the function to a std::string pointer
    std::string* myoutstring = (std::string*)userdata;

    //push back each byte of data to the output string
    for(size_t i=0; i < nmemb; ++i) {
        myoutstring->push_back(ptr[i]);
    }

    return nmemb;
}

std::string run_curl(CURL* curl, std::string target_url) {
    // CURL *curl;
    CURLcode res;

    std::string myoutstring;

    //remove spaces from the URL
    // replace_spaces(target_url, "%20");

    // curl = curl_easy_init();
    if(curl) {
        const char* target_c_url = target_url.c_str();
        char *output = curl_easy_escape(curl, target_c_url, target_url.length());

        //set URL for the handle
        curl_easy_setopt(curl, CURLOPT_URL, output);

        curl_free(output);

        //tell curl where to write data
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &myoutstring);

        //tell curl what to do with data we receive
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        //perform the operation specified
        res = curl_easy_perform(curl);

        //handle errors
        if(res != CURLE_OK) {
            std::cerr<<"Curl encountered an error. Ending process\n" << curl_easy_strerror(res);
            return "";
        }

        
    } else {
        std::cerr<<"Error in creating curl handler\n";
        return "";
    }

    return myoutstring;
}

std::vector<std::string> parse_neighbors(std::string const &json_string) {
    using namespace rapidjson;
    Document document;
    //convert our c++ string to a c string to pass to rapidjson
    document.Parse(json_string.c_str());

    if(document.HasMember("error")) {
        throw std::runtime_error(document["error"].GetString());
    }

    assert(document.IsObject());
    assert(document.HasMember("neighbors"));
    assert(document["neighbors"].IsArray());

    const Value& a = document["neighbors"];

    std::vector<std::string> neighbors;
    for(size_t i = 0; i < a.Size(); ++i) {
        assert(a[i].IsString());
        neighbors.push_back(a[i].GetString());
    }

    return neighbors;
}

class Node {
    public:
    std::string value;
    Node* parent;
    std::vector<Node> children;
    int depth;
    CURL* curl;

    Node(std::string value, int depth, CURL* curl_pointer) {
        this->value = value;
        this->depth = depth;
        this->curl = curl_pointer;
        parent = NULL;
    }

    bool value_exists(const std::string& v) {
        if(depth == 0 && value != v) {
            return false;
        }

        return (value == v) || parent->value_exists(v);
    }

    void create_child(std::string val) {
        //std::cout<<"Node "<<value<<" creating child "<<val<<", depth = "<<(this->depth+1)<<"\n";
        Node child = Node(val, this->depth + 1, this->curl);
        child.parent = this;
        this->children.push_back(child);
    }

    void create_children_from_db(const int& max_depth) {
        if(this->depth >= max_depth) return;

        std::string json_result = run_curl(this->curl, ENDPOINT + value);
        std::vector<std::string> neighbors = parse_neighbors(json_result);

        //create children for this node
        for(std::string& s: neighbors) {
            if(value_exists(s)) continue;

            this->create_child(s);
        }

        //have all children create their own trees
        for(Node& c: this->children) {
            c.create_children_from_db(max_depth);
        }
    }

    std::string to_string() {
        std::stringstream ret;
        ret<<value<<", depth="<<depth<<", num children="<<this->children.size()<<"\n";
        return ret.str();
    }

    void print_tree(std::string indent) {
        std::cout<<indent<<this->to_string();
        for(Node& c: this->children) {
            c.print_tree(indent + indent);
        }
    }

    void write_tree(std::ofstream& outstream, std::string indent) {
        outstream<<indent<<this->to_string();
        for(Node& c: this->children) {
            c.write_tree(outstream, indent + indent);
        }
    }
};

int main(int argc, char* argv[]) {
    //initialize command line args
    if(argc != 4){
        std::cout<<"Use the following arguments to run on command line:\n";
        std::cout<<"Arg 1: (string) Name of initial node. If the name contains spaces, you will need to use quotation marks\n";
        std::cout<<"Arg 2: (int) Maximum depth of search\n";
        std::cout<<"Arg 3: (string) Filepath of output (use .txt)\n";
        return 0;
    }

    std::string parent_value = argv[1];
    int max_depth = std::stoi(argv[2]);
    std::string output_filepath = argv[3];

    CURL* curl = curl_easy_init();

    Node parent = Node(parent_value, 0, curl);


    //time execution
    namespace chrn = std::chrono;
    auto start = chrn::high_resolution_clock::now();

    parent.create_children_from_db(max_depth);

    auto end = chrn::high_resolution_clock::now();
    auto elapsed_us = chrn::duration_cast<chrn::microseconds>(end - start).count();
    double elapsed_ms = elapsed_us / 1000.0;

    //cleanup
    curl_easy_cleanup(curl);


    //write output
    std::ofstream output_file;
    output_file.open(output_filepath, std::ios::out);
    output_file<<"Execution Time: "<<elapsed_ms<<"ms\n";

    parent.write_tree(output_file, "\t");
    output_file.close();

    return 0;
};