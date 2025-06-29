#include <curling/curling.hpp>
#include <iostream>
#include <string>
#include <sstream>

int main() {
    try {
        std::string token = "your_bearer_token_here";
        std::string baseURL = "https://api.example.com/v1/items";
        int currentPage = 1;
        int totalPages = 3; // assume we want to fetch 3 pages

        while (currentPage <= totalPages) {
            curling::Request req;
            req.setMethod(curling::Request::Method::GET)
               .setURL(baseURL)
               .addArg("page", std::to_string(currentPage)) // Adds ?page=1 to URL
               .setAuthToken(token)
               .addHeader("Accept: application/json")
               .setTimeout(10);

            curling::Response res = req.send();

            std::cout << "Page " << currentPage << ":\n";
            std::cout << res.toString() << "\n";

            currentPage++;
        }

    } catch (const curling::CurlingException& ex) {
        std::cerr << "Request failed: " << ex.what() << std::endl;
    }

    return 0;
}
