#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setURL("https://example.com")
       .setProxy("http://127.0.0.1:8080")
       .setProxyAuth("user", "password")
       .setProxyAuthMethod(curling::Request::AuthMethod::BASIC);

    curling::Response res = req.send();
    std::cout << res.httpCode;
}
