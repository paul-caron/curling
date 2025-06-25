#include "curling.hpp"

using namespace curling;
using namespace std;

int main(){

    Request request;

    request.setMethod(Request::Method::GET);
    request.setURL("https://httpbin.org/post");
    request.setUserAgent("Bond/James-Bond/007");
    request.addFormField("formKey", "formValue");
    request.addFormFile("file", "test.txt");
    request.setTimeout(10);
    request.setConnectTimeout(5);

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

