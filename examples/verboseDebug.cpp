#include "curling.hpp"

int main() {
    curling::Request req;
    req.setURL("https://example.com")
       .enableVerbose(true); // logs to stderr

    curling::Response res = req.send();
}
