#include <iostream>
#include <thread>
#include <vector>
#include "curling.hpp"

void downloadFile(const std::string& url, const std::string& outputFile) {
    try {
        curling::Request req;
        req.setMethod(curling::Request::Method::GET)
           .setURL(url)
           .downloadToFile(outputFile)
           .setTimeout(30);

        curling::Response res = req.send();

        std::cout << "Downloaded " << url << " with HTTP code: " << res.httpCode << std::endl;
        if (res.httpCode != 200) {
            std::cerr << "Warning: HTTP status code indicates error for URL " << url << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error downloading " << url << ": " << ex.what() << std::endl;
    }
}

int main() {
    // List of URLs and corresponding filenames
    std::vector<std::pair<std::string, std::string>> downloads = {
        {"https://example.com/file1.jpg", "file1.jpg"},
        {"https://example.com/file2.jpg", "file2.jpg"},
        {"https://example.com/file3.jpg", "file3.jpg"}
    };

    std::vector<std::thread> threads;

    // Launch a thread for each download
    for (const auto& [url, filename] : downloads) {
        threads.emplace_back(downloadFile, url, filename);
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "All downloads completed." << std::endl;
    return 0;
}
