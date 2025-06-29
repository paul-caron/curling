#include "curling.hpp"
#include <iostream>

int main() {
    // First request: sets cookies
    {
        curling::Request req;
        req.setURL("https://httpbin.org/cookies/set?session=abc123")
           .setCookiePath("session_cookies.txt");

        req.send();
    }

    // Second request: cookies persisted
    {
        curling::Request req;
        req.setURL("https://httpbin.org/cookies")
           .setCookiePath("session_cookies.txt");

        curling::Response res = req.send();
        std::cout << res.body;
    }
}
