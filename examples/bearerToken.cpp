#include <curling/curling.hpp>
#include <iostream>

int main() {
    try {
        curling::Request req;

        // Replace with your actual Bearer token
        std::string token = "your_actual_token_here";

        curling::Response res = req.setMethod(curling::Request::Method::GET)
            .setURL("https://api.example.com/v1/profile")
            .setAuthToken(token) // Adds "Authorization: Bearer <token>"
            .addHeader("Accept: application/json")
            .setTimeout(10)
            .send();

        std::cout << "Response:\n" << res.toString() << std::endl;

    } catch (const curling::CurlingException& ex) {
        std::cerr << "Request failed: " << ex.what() << std::endl;
    }

    return 0;
}
