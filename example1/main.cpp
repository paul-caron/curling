#include "curling.hpp"

using namespace curling;
using namespace std;


int main(){
    Request request;
    request.setMethod(Request::POST);
    request.setURL("https://httpbin.org/post");
    request.addHeader("Content-Type: application/json");
    string jsonBody = R"({"key": "value"})";
    request.setBody(jsonBody);
    request.send();
    cout << "Response Data:\n" << request.getResponse() << endl;

    // Access and print response headers stored in the map
    auto responseHeadersMap = request.getResponseHeadersMap();
    cout << "\nResponse Headers Map:" << endl;
    for (const auto& header : responseHeadersMap) {
        cout << header.first << ": " << header.second << endl;
    }

    return 0;
}
