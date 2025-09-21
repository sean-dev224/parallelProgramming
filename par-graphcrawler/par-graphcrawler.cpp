#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <queue>
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


void expand_nodes(CURL* curl, std::pair<int, int>& indices, std::vector<std::string>& current_level, std::vector<std::string>& next_level, std::unordered_set<std::string>& visited, std::mutex& mut) {
    auto id = std::this_thread::get_id();
    for(int i = indices.first; i <= indices.second; i++) {
        try {
            if (debug)
                std::cout<<"Thread "<< id << " Trying to expand "<<current_level[i]<<"\n";

            //for each new neighbor
            for (const auto& neighbor : get_neighbors(fetch_neighbors(curl, current_level[i]))) {
                if (debug)
                    std::cout<<"neighbor "<<neighbor<<"\n";

                std::lock_guard<std::mutex> lg(mut);
                if (!visited.count(neighbor)) {
                    visited.insert(neighbor);
                    next_level.push_back(neighbor);
                }

            }
        } catch (const ParseException& e) {
            std::cerr<<"Error while fetching neighbors of: "<<current_level[i]<<std::endl;
            throw e;
        }
    }
    
}

std::vector<std::pair<int, int>> split_list(const std::vector<std::string>& list, int n_segments) {
    //returns a vector containing start and end indexes for n sub-lists

    std::vector<std::pair<int, int>> result;
    int total = list.size();
    if (n_segments <= 0 || total == 0) return result;

    int base_size = total / n_segments;      // minimum number of items per segment
    int remainder = total % n_segments;      // leftover items to distribute

    int start = 0;
    for (int i = 0; i < n_segments; ++i) {
        int seg_size = base_size + (i < remainder ? 1 : 0); // spread remainder across first segments
        int end = start + seg_size - 1; // inclusive end index
        if (seg_size > 0) {
            result.emplace_back(start, end);
        }
        start = end + 1;
    }

    return result;
}



// BFS Traversal Function
std::vector<std::vector<std::string>> bfs(std::vector<CURL*>& curl_handles, const std::string& start, int depth) {
    std::vector<std::vector<std::string>> levels;
    std::unordered_set<std::string> visited;
    int max_threads = curl_handles.size();
    std::vector<std::thread> threadgroup;
    std::mutex mut;
    
    levels.push_back({start});
    visited.insert(start);

    for (int d = 0;  d < depth; d++) {
        if (debug)
            std::cout<<"starting level: "<<d<<"\n";
        
        levels.push_back({});


        std::vector<std::pair<int, int>> indices = split_list(levels[d], max_threads);
        for(int i = 0; i < indices.size(); i++) {
            threadgroup.push_back(std::thread(expand_nodes, curl_handles[i], std::ref(indices[i]), std::ref(levels[d]), std::ref(levels[d+1]), std::ref(visited), std::ref(mut)));
        }

        //recover all threads
        for(auto& t: threadgroup) {
            t.join();
        }
        threadgroup.clear();


    }
    
    return levels;
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




    const auto start{std::chrono::steady_clock::now()};
    
    
    for (const auto& n : bfs(curl_handles, start_node, depth)) {
        for (const auto& node : n)
	        std::cout << "- " << node << "\n";
        std::cout<<n.size()<<"\n";
    }
    
    const auto finish{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{finish - start};
    std::cout << "Time to crawl: "<<elapsed_seconds.count() << "s\n";
    
    //cleanup all the handles
    for(CURL* curl: curl_handles) {
        curl_easy_cleanup(curl);
    }
    
    return 0;
}
