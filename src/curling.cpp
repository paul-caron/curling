#include "curling.hpp"

namespace curling {

namespace {

std::once_flag curlGlobalInitFlag;
std::mutex curlGlobalMutex;

int instanceCount = 0;

void ensureCurlGlobalInit(){
    std::call_once(curlGlobalInitFlag, []{
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
    std::lock_guard<std::mutex> lock(curlGlobalMutex);
    ++instanceCount;
}

void maybeCleanupGlobalCurl(){
    std::lock_guard<std::mutex> lock(curlGlobalMutex);
    if(--instanceCount==0){
        curl_global_cleanup();
    }
}

}//anonymous namespace end

Request::Request() : curlHandle(nullptr), list(nullptr), cookieFile("cookies.txt"), cookieJar("cookies.txt") {
    ensureCurlGlobalInit();
    
    curlHandle.reset(curl_easy_init());
    if (!curlHandle) {
        throw std::runtime_error("Curl initialization failed");
    }

    //setup cookies
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEJAR, cookieJar.c_str());

    //set default method
    curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1L);
}

Request::Request(Request&& other) noexcept
   :method(other.method),
    curlHandle(other.curlHandle),
    list(other.list),
    url(std::move(other.url)),
    args(std::move(other.args)),
    body(std::move(other.body)),
    cookieFile(std::move(other.cookieFile)),
    cookieJar(std::move(other.cookieJar)),
    mime(std::move(other.mime)){
    other.curlHandle = nullptr;
    other.list = nullptr;
    other.mime = nullptr;
}

Request& Request::operator=(Request&& other) noexcept {
    if(this != &other) {
        clean();

        //transfer ownership
        method = other.method;
        curlHandle = other.curlHandle;
        list = other.list;
        mime = other.mime;

        url = std::move(other.url);
        args = std::move(other.args);
        body = std::move(other.body);
        cookieFile = std::move(other.cookieFile);
        cookieJar = std::move(other.cookieJar);

        other.curlHandle = nullptr;
        other.list = nullptr;
        other.mime = nullptr;
    }
    return *this;
}

Request::~Request() {
    clean();
    maybeCleanupGlobalCurl();
}

Request& Request::setMethod(Method m) {
    if(method == Method::MIME && m!= Method::MIME){
        throw std::logic_error("Cannot override MIME method with another HTTP method");
    }
    
    //reset CUSTOMREQUEST and others so they dont interfere with each other when curl sends request
    curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 0L);
    curl_easy_setopt(curlHandle, CURLOPT_POST, 0L);
    curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, nullptr);
    
    method = m;
    
    switch (method) {
        case Method::MIME: break;
        case Method::GET:
            curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1L);
            break;
        case Method::POST:
            curl_easy_setopt(curlHandle, CURLOPT_POST, 1L);
            break;
        case Method::PUT:
            curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case Method::PATCH:
            curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case Method::DEL:
            curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
    }
    return *this;
}

Request& Request::setURL(const std::string& URL) {
    url = URL;
    return *this;
}

Request& Request::addArg(const std::string& key, const std::string& value) {
    char* escapedKey = curl_easy_escape(curlHandle, key.c_str(), 0);
    char* escapedValue = curl_easy_escape(curlHandle, value.c_str(), 0);

    if(escapedKey && escapedValue){
        std::string arg = std::string(escapedKey) + "=" + escapedValue;
        args.append(args.empty() ? "" : "&").append(arg);
    }
    
    if(escapedKey) curl_free(escapedKey);
    if(escapedValue) curl_free(escapedValue);
    return *this;
}

Request& Request::addHeader(const std::string& header) {
    if (list) {
        list = curl_slist_append(list, header.c_str());
    } else {
        list = curl_slist_append(nullptr, header.c_str());
    }
    curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, list);
    return *this;
}

Request& Request::setBody(const std::string& body) {
    this->body = body;
    if (method == Method::POST || method == Method::PUT || method == Method::PATCH) {
        curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(this->body.size()));
        curl_easy_setopt(curlHandle, CURLOPT_COPYPOSTFIELDS, this->body.data());
    }
    return *this;
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

    reset();
    
    return response;
}

void Request::reset() {
    clean();
    curlHandle.reset(curl_easy_init());
    if(!curlHandle){
        throw std::runtime_error("Curl re-initialization failed");
    }
    
    args.clear();
    url.clear();
    body.clear();
    method = Method::GET;

    curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEJAR, cookieJar.c_str());
}

void Request::clean() {
    if (mime) {
        curl_mime_free(mime);
        mime = nullptr;
    }
    if (list) {
        curl_slist_free_all(list);
        list = nullptr;
    }
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

Request& Request::setTimeout(long seconds){
    curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, seconds);
    return *this;
}

Request& Request::setProxy(const std::string& url){
    curl_easy_setopt(curlHandle, CURLOPT_PROXY, url.c_str());
    return *this;
}

Request& Request::setProxyAuth(const std::string& username, const std::string & password){
    curl_easy_setopt(curlHandle, CURLOPT_PROXYUSERPWD, (username+":"+password).c_str());
    return *this;
}

Request& Request::setProxyAuthMethod(long method){
    //example: CURLAUTH_BASIC, CURLAUTH_NTLM, CURLAUTH_DIGEST, CURLAUTH_ANY
    curl_easy_setopt(curlHandle, CURLOPT_PROXYAUTH, method);
    return *this;
}

Request& Request::setConnectTimeout(long seconds){
    curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, seconds);
    return *this;
}

Request& Request::setAuthToken(const std::string& token){
    std::string header = "Authorization: Bearer " + token;
    addHeader(header);
    return *this;
}

Request& Request::setFollowRedirects(bool follow){
    curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
    return *this;
}

Request& Request::setCookiePath(const std::string& path){
    //set path to read cookies from
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEFILE, path.c_str());
    //set path to write cookies to
    curl_easy_setopt(curlHandle, CURLOPT_COOKIEJAR, path.c_str());
    //set member variables
    cookieFile = path;
    cookieJar = path;
    return *this;
}

Request& Request::setUserAgent(const std::string& userAgent){
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, userAgent.c_str());
    return *this;
}

Request& Request::addFormField(const std::string& fieldName, const std::string & value){
    if(!mime){
        mime = curl_mime_init(curlHandle);
        if(!mime) throw std::runtime_error("Failed to initialize MIME");
        curl_easy_setopt(curlHandle, CURLOPT_MIMEPOST, mime);
    }
    curl_mimepart* part = curl_mime_addpart(mime);
    if(!part) throw std::runtime_error("Failed to add MIME part");
    curl_mime_name(part, fieldName.c_str());
    curl_mime_data(part, value.c_str(), CURL_ZERO_TERMINATED);
    return *this;
}

Request& Request::addFormFile(const std::string& fieldName, const std::string & filePath){
    if(!mime){
        mime = curl_mime_init(curlHandle);
        if(!mime) throw std::runtime_error("Failed to initialize MIME");
        curl_easy_setopt(curlHandle, CURLOPT_MIMEPOST, mime);
    }
    curl_mimepart* part = curl_mime_addpart(mime);
    if(!part) throw std::runtime_error("Failed to add MIME part");
    curl_mime_name(part, fieldName.c_str());
    curl_mime_filedata(part, filePath.c_str());
    return *this;
}

Request& Request::enableVerbose(bool enabled){
    curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, enabled ? 1L : 0L);
    return *this;
}

} // namespace curling
