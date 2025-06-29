#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setURL("https://httpbin.org/get")
       .setHttpVersion(curling::Request::HttpVersion::HTTP_2); // or HTTP_3

    curling::Response res = req.send();
    std::cout << res.httpCode << "\n" << res.body;
}
