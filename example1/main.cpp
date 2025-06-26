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

    cout << "Response Code: " << response.httpCode << endl;
    cout << "Response Data:\n" << response.body << endl;

    auto responseHeadersMap = response.headers;
    cout << "\nResponse Headers Map:" << endl;
    for (const auto& header : responseHeadersMap) {
        cout << header.first << ": " << header.second[0] << endl;
    }

    return 0;

}
