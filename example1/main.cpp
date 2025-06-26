#include "curling.hpp"

using namespace curling;
using namespace std;

int main(){

    Request request;

    request.setMethod(Request::Method::POST)
           .setURL("https://httpbin.org/post")
           .setUserAgent("Bond/James-Bond/007")
           .addHeader("Content-Type: application/json")
           .setBody(R"({"jsonKey": "jsonValue"})")
           .addArg("argKey","argValue");

    auto response = request.send();

    cout << response.toString();

    return 0;

}
