#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/get")
       .addArg("foo", "bar")
       .addHeader("Accept: application/json");

    curling::Response res = req.send();
    std::cout << res.toString();
}
