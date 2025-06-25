#include "curling.hpp"

namespace curling {

Request::Request() : curlHandle(curl_easy_init()), list(nullptr), cookieFile("cookies.txt"), cookieJar("cookies.txt") {
    if (!curlHandle) {
        throw std::runtime_error("Curl initialization failed");
    }
    
    if (instances++ == 0){
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEJAR, cookieJar.c_str());
}

Request::~Request() {
    clean();
    if (--instances == 0)
        curl_global_cleanup();

    if (list) {
        curl_slist_free_all(list);
        list = nullptr;
    }
}

void Request::setMethod(Method m) {
    method = m;
    switch (method) {
        case Method::GET:
            curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1L);
            break;
        case Method::POST:
            curl_easy_setopt(curlHandle, CURLOPT_POST, 1L);
            break;
        case Method::PUT:
            curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case Method::DELETE:
            curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
    }
}

void Request::setURL(const std::string& URL) {
    url = URL;
}

void Request::addArg(const std::string& arg) {
    args.append(args.empty() ? "" : "&").append(arg);
}

void Request::addHeader(const std::string& header) {
    if (list) {
        list = curl_slist_append(list, header.c_str());
    } else {
        list = curl_slist_append(nullptr, header.c_str());
    }
    curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, list);
}

void Request::setBody(const std::string& body) {
    this->body = body;
    if (method == Method::POST || method == Method::PUT) {
        curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(this->body.size()));
        curl_easy_setopt(curlHandle, CURLOPT_COPYPOSTFIELDS, this->body.data());
    }
}

size_t Request::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto responseStream = static_cast<std::ostringstream*>(userp);
    responseStream->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

size_t Request::HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* headerMap = static_cast<std::map<std::string, std::vector<std::string>>*>(userdata);
    std::string headerLine(buffer, size * nitems);

    if (headerLine.empty()) return 0; // skip the separation line

    auto colonPos = headerLine.find(":");
    if (colonPos != std::string::npos) {
        std::string key = headerLine.substr(0, colonPos);
        std::string value = headerLine.substr(colonPos + 1);
        Request::trim(key);
        Request::trim(value);
        (*headerMap)[key].push_back(value);
    }

    return size * nitems;
}

Response Request::send() {
    std::ostringstream responseStream;
    Response response;

    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &responseStream);

    // Use responseHeadersMap to store headers
    curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curlHandle, CURLOPT_HEADERDATA, &(response.headers));
    
    updateURL();
    CURLcode res = curl_easy_perform(curlHandle);
    if (res != CURLE_OK) {
        throw std::runtime_error("Curl perform failed: " + std::string(curl_easy_strerror(res)));
    }
 
    curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &(response.httpCode)); // store the http code from the response

    response.body = responseStream.str(); // Store the response body
    return response;
}

void Request::reset() {
    if(curlHandle){
        curl_easy_reset(curlHandle);
    }
    curlHandle = curl_easy_init();
    
    args.clear();
    url.clear();
    body.clear();

    if (list) {
        curl_slist_free_all(list);
        list = nullptr;
    }
    
    method = Method::GET;
    curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEJAR, cookieJar.c_str());
}

void Request::clean() {
    if (curlHandle) {
        curl_easy_cleanup(curlHandle);
        curlHandle = nullptr;
    }
}

void Request::updateURL() {
    std::string s = args.empty() ? url : url + "?" + args;
    curl_easy_setopt(curlHandle, CURLOPT_URL, s.c_str());
}

void Request::trim(std::string &s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){
        return !std::isspace(ch);
    }).base(), s.end());
}

} // namespace curling
