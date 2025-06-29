#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setMethod(curling::Request::Method::POST)
       .setURL("https://httpbin.org/post")
       .addHeader("Content-Type: application/json")
       .setBody(R"({"message": "hello"})");

    curling::Response res = req.send();
    std::cout << res.body << std::endl;
}
