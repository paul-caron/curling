#ifndef CURLING_HPP
#define CURLING_HPP

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <curl/curl.h>

namespace curling {

class Request {
public:
    enum Method { GET, POST, PUT, DELETE };

    static int instances;

    Request();
    ~Request();

    void setMethod(Method m);
    void setURL(const std::string& URL);
    void addArg(const std::string& arg);
    void addHeader(const std::string& header);
    void setBody(const std::string& body);

    void send();
    const std::string& getResponse() const;
    const std::map<std::string, std::string>& getResponseHeadersMap() 
const;

    void reset();

private:
    Method method;
    CURL* curlHandle;
    struct curl_slist* list;
    std::string url, args, response, body, cookieFile, cookieJar;
    std::map<std::string, std::string> responseHeadersMap;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, 
void* userp);
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, 
void* userdata);

    void clean();
    void updateURL();
};

} // namespace curling

#endif // CURLING_HPP
