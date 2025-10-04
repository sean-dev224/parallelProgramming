#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <queue>
#include <condition_variable>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <stdexcept>
#include "rapidjson/error/error.h"
#include "rapidjson/reader.h"


struct ParseException : std::runtime_error, rapidjson::ParseResult {
    ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset) : 
        std::runtime_error(msg), 
        rapidjson::ParseResult(code, offset) {}
};

#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset) \
    throw ParseException(code, #code, offset)

#include <rapidjson/document.h>
#include <chrono>


bool debug = true;

// Updated service URL
const std::string SERVICE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

template <typename T>
class BlockingQueue {
    std::queue<T> q;
    std::mutex mut;
    std::condition_variable queue_not_empty;
    bool processing_complete = false;

    public:
    

    BlockingQueue() {
    }

    void push(const T& element) {
        std::unique_lock<std::mutex> lg(mut);
        q.push(element);
        queue_not_empty.notify_one();
    }

    bool pop(T& t) {
        std::unique_lock<std::mutex> lg(mut);
        
        //thread can be woken up spuriously, so the functor checks whether it should actually be awake every time it tries
        queue_not_empty.wait(lg, [this]{ return !q.empty() || this->processing_complete; });

        if(processing_complete) return false;

        t = q.front();
        q.pop();
        return true;
    }

    void finish() {
        std::unique_lock<std::mutex> lg(mut);
        processing_complete = true;
        queue_not_empty.notify_all();

    }
};

class Node {
    public:
    std::string value;
    int depth;

    Node() {}

    Node(std::string value, int depth) {
        this->value = value;
        this-> depth = depth;
    }

    bool operator== (const Node& node2) const {
        return value == node2.value;
    }
};

//the unordered set requires a hash function to be defined for an object to be used in it
namespace std {
    template <>
    struct hash<Node> {
        std::size_t operator()(const Node& n) const noexcept {
            // use the existing string hasher
            return std::hash<std::string>()(n.value);
        }
    };
}

// Function to HTTP ecnode parts of URLs. for instance, replace spaces with '%20' for URLs
std::string url_encode(CURL* curl, std::string input) {
  char* out = curl_easy_escape(curl, input.c_str(), input.size());
  std::string s = out;
  curl_free(out);
  return s;
}

// Callback function for writing response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Function to fetch neighbors using libcurl with debugging
std::string fetch_neighbors(CURL* curl, const std::string& node) {

  std::string url = SERVICE_URL + url_encode(curl, node);
  std::string response;

    if (debug)
      std::cout << "Sending request to: " << url << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Verbose Logging

    // Set a User-Agent header to avoid potential blocking by the server
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: C++-Client/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    } else {
      if (debug)
        std::cout << "CURL request successful!" << std::endl;
    }

    // Cleanup
    curl_slist_free_all(headers);

    if (debug) 
      std::cout << "Response received: " << response << std::endl;  // Debug log

    return (res == CURLE_OK) ? response : "{}";
}

// Function to parse JSON and extract neighbors
std::vector<std::string> get_neighbors(const std::string& json_str) {
    std::vector<std::string> neighbors;
    try {
      rapidjson::Document doc;
      doc.Parse(json_str.c_str());
      
      if (doc.HasMember("neighbors") && doc["neighbors"].IsArray()) {
        for (const auto& neighbor : doc["neighbors"].GetArray())
	        neighbors.push_back(neighbor.GetString());
      }
    } catch (const ParseException& e) {
      std::cerr<<"Error while parsing JSON: "<<json_str<<std::endl;
      throw e;
    }
    return neighbors;
}



void expand_nodes(CURL* curl, BlockingQueue<Node>& q, std::unordered_set<Node>& visited, std::mutex& visited_mutex, const int& max_depth) {
    auto id = std::this_thread::get_id();

    while(true) {
        Node to_expand;

        if(!q.pop(to_expand))
            break;

        if(to_expand.depth >= max_depth)
            continue;


        try {
            if (debug)
                std::cout<<"Thread "<< id << " Trying to expand "<<to_expand.value<<"\n";

            //for each new neighbor
            std::lock_guard<std::mutex> lg(visited_mutex);
            for (const auto& neighbor : get_neighbors(fetch_neighbors(curl, to_expand.value))) {
                if (debug)
                    std::cout<<"neighbor "<<neighbor<<"\n";

                Node neighbor_node = Node(neighbor, to_expand.depth+1);
                if (!visited.count(neighbor_node)) {
                    visited.insert(neighbor_node);
                    q.push(neighbor_node);
                }

            }
        } catch (const ParseException& e) {
            std::cerr<<"Error while fetching neighbors of: "<<to_expand.value<<std::endl;
            throw e;
        }

    }
}


// BFS Traversal Function
std::unordered_set<Node> bfs(std::vector<CURL*>& curl_handles, const std::string& start, int depth) {
    int max_threads = curl_handles.size();
    std::vector<std::thread> threadgroup;
    std::unordered_set<Node> visited;
    std::mutex visited_mutex;
    BlockingQueue<Node> q;

    //add first node
    q.push(Node(start, 0));

    for (CURL* handle: curl_handles) {
        threadgroup.push_back(std::thread(expand_nodes, handle, std::ref(q), std::ref(visited), std::ref(visited_mutex), std::ref(depth)));
    }

    std::this_thread::sleep_for(std::chrono::seconds(15));
    std::cout << "Finished waiting\n";
    q.finish();

    //recover all threads
    for(auto& t: threadgroup) {
        t.join();
    }
    
    return visited;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <node_name> <depth> <max_threads>\n";
        return 1;
    }

    std::string start_node = argv[1];     // example "Tom%20Hanks"
    int depth;
    try {
        depth = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Depth must be an integer.\n";
        return 1;
    }
    int max_threads;
    try {
        max_threads = std::stoi(argv[3]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Max threads must be an integer.\n";
        return 1;
    }

    //create a curl handle for each thread
    std::vector<CURL*> curl_handles;
    for(int i = 0; i < max_threads; i++) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL\n";
            return -1;
        }
        curl_handles.push_back(curl);
    }



    //time execution
    const auto start{std::chrono::steady_clock::now()};
    
    std::unordered_set<Node> nodes= bfs(curl_handles, start_node, depth);
    
    const auto finish{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{finish - start};

    //print results
    for (const auto& node : nodes) {
	    std::cout << "- " << node.value << "\n";
    }
    std::cout<<nodes.size()<<"\n";
    std::cout << "Time to crawl: "<<elapsed_seconds.count() << "s\n";
    
    //cleanup all the handles
    for(CURL* curl: curl_handles) {
        curl_easy_cleanup(curl);
    }
    
    return 0;
}
