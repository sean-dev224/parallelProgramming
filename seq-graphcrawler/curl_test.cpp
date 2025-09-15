#include <curl/curl.h>
#include <string>
#include <iostream>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    //cast the void pointer from the function to a std::string pointer
    std::string* myoutstring = (std::string*)userdata;

    //push back each byte of data to the output string
    for(size_t i=0; i < nmemb; ++i) {
        myoutstring->push_back(ptr[i]);
    }

    return nmemb;
}

int main() {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        //set URL for the handle
        curl_easy_setopt(curl, CURLOPT_URL, "https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html");

        //tell curl where to write data
        std::string myoutstring;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &myoutstring);

        //tell curl what to do with data we receive
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        //perform the operation specified
        res = curl_easy_perform(curl);

        //handle errors
        if(res != CURLE_OK) {
            std::cerr<<"Curl encountered an error. Ending process\n" << curl_easy_strerror(res);
            return 0;
        }

        std::cout<<myoutstring;

        //cleanup
        curl_easy_cleanup(curl);
    }
    return 0;
}