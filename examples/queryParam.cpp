#include "curling.hpp"
#include <iostream>
#include <string>

int main() {
    try {
        std::string token = "your_bearer_token_here";
        std::string baseURL = "https://api.example.com/v1/items";
        int totalPages = 3; // Total pages to fetch

        for (int currentPage = 1; currentPage <= totalPages; ++currentPage) {
            curling::Request req;
            req.setMethod(curling::Request::Method::GET)
               .setURL(baseURL)
               .addArg("page", std::to_string(currentPage)) // Appends ?page=1, ?page=2, etc.
               .setAuthToken(token)
               .addHeader("Accept: application/json")
               .setTimeout(10);

            curling::Response res = req.send();

            std::cout << "Page " << currentPage << ":\n";
            std::cout << res.toString() << "\n";
        }

    } catch (const curling::CurlingException& ex) {
        std::cerr << "Request failed: " << ex.what() << std::endl;
    }

    return 0;
}
