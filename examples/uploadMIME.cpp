#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setMethod(curling::Request::Method::MIME)
       .setURL("https://httpbin.org/post")
       .addFormField("field1", "value1")
       .addFormFile("file1", "example.txt");

    curling::Response res = req.send();
    std::cout << res.body;
}
