#include <iostream>
#include <sstream>
#include <string>
#include <curl/curl.h>

class Request {
public:
    enum Method { GET, POST, PUT, DELETE };

    static int instances;

    Request() : curlHandle(curl_easy_init()), list(nullptr) {
        if (instances++ == 0)
            curl_global_init(CURL_GLOBAL_DEFAULT);

        if (!curlHandle) {
            std::cerr << "Curl initialization failed!" << std::endl;
            throw std::runtime_error("Curl initialization failed");
        }
    }

    ~Request() {
        clean();
        if (--instances == 0)
            curl_global_cleanup();

        if (list) {
            curl_slist_free_all(list);
            list = nullptr;
        }
    }

    void setMethod(Method m) {
        method = m;
        switch (method) {
            case Method::GET:
                curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1L);
                break;
            case Method::POST:
                curl_easy_setopt(curlHandle, CURLOPT_POST, 1L);
                break;
            case Method::PUT:
                curl_easy_setopt(curlHandle, CURLOPT_UPLOAD, 1L);
                break;
            case Method::DELETE:
                curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
        }
    }

    void setURL(const std::string& URL) {
        url = URL;
        updateURL();
    }

    void addArg(const std::string& arg) {
        args.append(args.empty() ? "" : "&").append(arg);
        updateURL();
    }

    void addHeader(const std::string& header) {
        if (list) {
            list = curl_slist_append(list, header.c_str());
        } else {
            list = curl_slist_append(nullptr, header.c_str());
        }
        curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, list);
    }

    void setBody(const std::string& body) {
        this->body = body;
        if (method == Method::POST || method == Method::PUT) {
            curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
            curl_easy_setopt(curlHandle, CURLOPT_COPYPOSTFIELDS, body.c_str());
        }
    }

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        auto responseStream = static_cast<std::ostringstream*>(userp);
        responseStream->write(static_cast<char*>(contents), size * nmemb);
        return size * nmemb;
    }

    void send() {
        std::ostringstream responseStream;
        curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &responseStream);

        CURLcode res = curl_easy_perform(curlHandle);
        if (res != CURLE_OK) {
            std::cerr << "Curl perform failed: " << curl_easy_strerror(res) << std::endl;
            throw std::runtime_error("Curl perform failed");
        }

        long http_code = 0;
        curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &http_code);
        std::cout << "HTTP Response Code: " << http_code << std::endl;

        response = responseStream.str(); // Store the response
    }

    const std::string& getResponse() const {
        return response;
    }

    void reset() {
        curl_easy_reset(curlHandle);
        args.clear();
        if (list) {
            curl_slist_free_all(list);
            list = nullptr;
        }
        updateURL();  // Refresh the URL to ensure it's clean
    }

private:
    Method method = GET;
    std::string url, args, response, body;
    CURL* curlHandle = nullptr;
    struct curl_slist* list = nullptr;

    void clean() {
        if (curlHandle) {
            curl_easy_cleanup(curlHandle);
            curlHandle = nullptr;
        }
    }

    void updateURL() {
        std::string s = args.empty() ? url : url + "?" + args;
        curl_easy_setopt(curlHandle, CURLOPT_URL, s.c_str());
    }
};

int Request::instances = 0;

using namespace std;

int main() {
    try {
        Request request;
        request.setMethod(Request::POST);
        request.setURL("https://httpbin.org/post");
        request.addHeader("Content-Type: application/json");

        // Example JSON body
        string jsonBody = R"({"key": "value"})";
        request.setBody(jsonBody);

        request.send();
        cout << "Response Data:\n" << request.getResponse() << endl;
    } catch (const std::exception& e) {
        cerr << "Exception caught in main: " << e.what() << endl;
    }

    return 0;
}
