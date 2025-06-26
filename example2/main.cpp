#include "curling.hpp"

using namespace curling;
using namespace std;

int main(){

    Request request;

    request.setMethod(Request::Method::GET)
           .setURL("https://httpbin.org/post")
           .setUserAgent("Bond/James-Bond/007")
           .addFormField("formKey", "formValue")
           .addFormFile("file", "test.txt")
           .setTimeout(10)
           .setConnectTimeout(5);

    auto response = request.send();

    cout << response.toString();

    return 0;

}
