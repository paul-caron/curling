#include "curling.hpp"

using namespace curling;
using namespace std;

int main(){

    Request request;

    request.setMethod(Request::Method::MIME)
           .setURL("https://httpbin.org/post")
           .setUserAgent("Bond/James-Bond/007")
           .addFormField("formKey", "formValue")
           .addFormFile("file", "test.txt")
           .setConnectTimeout(5);
           .setTimeout(10)
           

    auto response = request.send();

    cout << response.toString();

    return 0;

}
