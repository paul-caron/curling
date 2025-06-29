#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setURL("https://httpbin.org/basic-auth/user/pass")
       .setHttpAuth("user", "pass")
       .setHttpAuthMethod(curling::Request::AuthMethod::BASIC);

    curling::Response res = req.send();
    std::cout << res.body;
}
