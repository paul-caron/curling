#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://example.com/image.jpg")
       .downloadToFile("image.jpg")
       .setTimeout(30);

    curling::Response res = req.send();
    std::cout << "Downloaded with HTTP " << res.httpCode << std::endl;
}
