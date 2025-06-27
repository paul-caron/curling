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

void maybeCleanupGlobalCurl() noexcept {
    std::lock_guard<std::mutex> lock(curlGlobalMutex);
    if(--instanceCount==0){
        curl_global_cleanup();
    }
}

}//anonymous namespace end

Request::Request() : method(Method::GET), curlHandle(nullptr), list(nullptr), cookieFile("cookies.txt"), cookieJar("cookies.txt") {
    ensureCurlGlobalInit();
    
    curlHandle.reset(curl_easy_init());
    if (!curlHandle) {
        throw InitializationException("Curl initialization failed");
    }

    //setup cookies
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEJAR, cookieJar.c_str());

    //set default method
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPGET, 1L);
}

Request::Request(Request&& other) noexcept
   :method(other.method),
    curlHandle(std::move(other.curlHandle)),
    list(std::move(other.list)),
    url(std::move(other.url)),
    args(std::move(other.args)),
    body(std::move(other.body)),
    cookieFile(std::move(other.cookieFile)),
    cookieJar(std::move(other.cookieJar)),
    mime(std::move(other.mime)){
}

Request& Request::operator=(Request&& other) noexcept {
    if(this != &other) {
        clean();

        //transfer ownership
        method = other.method;
        curlHandle = std::move(other.curlHandle);
        list = std::move(other.list);
        mime = std::move(other.mime);

        url = std::move(other.url);
        args = std::move(other.args);
        body = std::move(other.body);
        cookieFile = std::move(other.cookieFile);
        cookieJar = std::move(other.cookieJar);
    }
    return *this;
}

Request::~Request() noexcept {
    clean();
    maybeCleanupGlobalCurl();
}

Request& Request::setMethod(Method m) {
    if(method == Method::MIME && m!= Method::MIME){
        throw LogicException("Cannot override MIME method with another HTTP method");
    }
    
    //reset CUSTOMREQUEST and others so they dont interfere with each other when curl sends request
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPGET, 0L);
    curl_easy_setopt(curlHandle.get(), CURLOPT_POST, 0L);
    curl_easy_setopt(curlHandle.get(), CURLOPT_CUSTOMREQUEST, nullptr);
    
    method = m;
    
    switch (method) {
        case Method::MIME: break;
        case Method::GET:
            curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPGET, 1L);
            break;
        case Method::POST:
            curl_easy_setopt(curlHandle.get(), CURLOPT_POST, 1L);
            break;
        case Method::PUT:
            curl_easy_setopt(curlHandle.get(), CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case Method::PATCH:
            curl_easy_setopt(curlHandle.get(), CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
        case Method::DEL:
            curl_easy_setopt(curlHandle.get(), CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
    }
    return *this;
}

Request& Request::setURL(const std::string& URL) {
    url = URL;
    return *this;
}

Request& Request::addArg(const std::string& key, const std::string& value) {
    char* escapedKey = curl_easy_escape(curlHandle.get(), key.c_str(), 0);
    char* escapedValue = curl_easy_escape(curlHandle.get(), value.c_str(), 0);

    if(escapedKey && escapedValue){
        std::string arg = std::string(escapedKey) + "=" + escapedValue;
        args.append(args.empty() ? "" : "&").append(arg);
    }
    
    if(escapedKey) curl_free(escapedKey);
    if(escapedValue) curl_free(escapedValue);
    return *this;
}

Request& Request::addHeader(const std::string& header) {
    auto newList = curl_slist_append(list.get(), header.c_str());
    if(!newList){
        throw HeaderException("Failed to append header to curl_slist");
    }
    list.release(); //release is needed here to avoid double free, as newList will contain this old pointer somewhere down the list chain
    list.reset(newList);
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPHEADER, list.get());
    return *this;
}

Request& Request::downloadToFile(const std::string& path) {
    downloadFilePath = path;
    return *this;
}

Request& Request::setBody(const std::string& body) {
    this->body = body;
    if (method == Method::POST || method == Method::PUT || method == Method::PATCH) {
        curl_easy_setopt(curlHandle.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(this->body.size()));
        curl_easy_setopt(curlHandle.get(), CURLOPT_COPYPOSTFIELDS, this->body.data());
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
        Request::toLowerCase(key);
        (*headerMap)[key].push_back(value);
    }

    return size * nitems;
}

int Request::ProgressCallbackBridge(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                    curl_off_t ultotal, curl_off_t ulnow) {
    auto* req = static_cast<Request*>(clientp);
    if (req->progressCallback) {
        bool shouldCancel = req->progressCallback(dltotal, dlnow, ultotal, ulnow);
        return shouldCancel ? 1 : 0; // Returning non-zero aborts transfer
    }
    return 0;
}

Response Request::send() {
    Response response;

    FILE* fileOut = nullptr;
    std::ostringstream responseStream;

    if (progressCallback) {
        curl_easy_setopt(curlHandle.get(), CURLOPT_XFERINFOFUNCTION, &Request::ProgressCallbackBridge);
        curl_easy_setopt(curlHandle.get(), CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curlHandle.get(), CURLOPT_NOPROGRESS, 0L); // must disable this
    }

    if (!downloadFilePath.empty()) {
        fileOut = std::fopen(downloadFilePath.c_str(), "wb");
        if (!fileOut) {
            throw RequestException("Failed to open file for writing: " + downloadFilePath);
        }
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, fileOut);
    } else {
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, &responseStream);
    }

    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERDATA, &(response.headers));

    updateURL();
    CURLcode res = curl_easy_perform(curlHandle.get());

    if (fileOut) std::fclose(fileOut);

    if (res != CURLE_OK) {
        throw RequestException("Curl perform failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_getinfo(curlHandle.get(), CURLINFO_RESPONSE_CODE, &(response.httpCode));

    if (downloadFilePath.empty()) {
        response.body = responseStream.str();
    }

    reset();

    return response;
}

/*

Response Request::send() {
    std::ostringstream responseStream;
    Response response;

    curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, &responseStream);

    // Use responseHeadersMap to store headers
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERDATA, &(response.headers));
    
    updateURL();
    CURLcode res = curl_easy_perform(curlHandle.get());
    if (res != CURLE_OK) {
        throw RequestException("Curl perform failed: " + std::string(curl_easy_strerror(res)));
    }
 
    curl_easy_getinfo(curlHandle.get(), CURLINFO_RESPONSE_CODE, &(response.httpCode)); // store the http code from the response

    response.body = responseStream.str(); // Store the response body

    reset();
    
    return response;
}
*/

void Request::reset() {
    CurlPtr newHandle(curl_easy_init());
    if(!newHandle){
        throw InitializationException("Curl re-initialization failed");
    }
    
    clean();
    curlHandle = std::move(newHandle);
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEJAR, cookieJar.c_str());
    
    args.clear();
    url.clear();
    body.clear();
    downloadFilePath.clear();
    method = Method::GET;

}

void Request::clean() noexcept {
    mime.reset();
    list.reset();
    curlHandle.reset();
}

void Request::updateURL() {
    std::string s = args.empty() ? url : url + "?" + args;
    curl_easy_setopt(curlHandle.get(), CURLOPT_URL, s.c_str());
}

void Request::trim(std::string &s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){
        return !std::isspace(ch);
    }).base(), s.end());
}

void Request::toLowerCase(std::string & s){
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

Request& Request::setTimeout(long seconds){
    curl_easy_setopt(curlHandle.get(), CURLOPT_TIMEOUT, seconds);
    return *this;
}

Request& Request::setProxy(const std::string& url){
    curl_easy_setopt(curlHandle.get(), CURLOPT_PROXY, url.c_str());
    return *this;
}

Request& Request::setProxyAuth(const std::string& username, const std::string & password){
    curl_easy_setopt(curlHandle.get(), CURLOPT_PROXYUSERPWD, (username+":"+password).c_str());
    return *this;
}

Request& Request::setProxyAuthMethod(AuthMethod method){
    //example: CURLAUTH_BASIC, CURLAUTH_NTLM, CURLAUTH_DIGEST
    curl_easy_setopt(curlHandle.get(), CURLOPT_PROXYAUTH, method);
    return *this;
}

Request& Request::setHttpAuth(const std::string& username, const std::string & password){
    curl_easy_setopt(curlHandle.get(), CURLOPT_USERPWD, (username+":"+password).c_str());
    return *this;
}

Request& Request::setHttpAuthMethod(AuthMethod method){
    //example: CURLAUTH_BASIC, CURLAUTH_NTLM, CURLAUTH_DIGEST
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPAUTH, method);
    return *this;
}

Request& Request::setConnectTimeout(long seconds){
    curl_easy_setopt(curlHandle.get(), CURLOPT_CONNECTTIMEOUT, seconds);
    return *this;
}

Request& Request::setAuthToken(const std::string& token){
    std::string header = "Authorization: Bearer " + token;
    addHeader(header);
    return *this;
}

Request& Request::setFollowRedirects(bool follow){
    curl_easy_setopt(curlHandle.get(), CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
    return *this;
}

Request& Request::setCookiePath(const std::string& path){
    //set path to read cookies from
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEFILE, path.c_str());
    //set path to write cookies to
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEJAR, path.c_str());
    //set member variables
    cookieFile = path;
    cookieJar = path;
    return *this;
}

Request& Request::setUserAgent(const std::string& userAgent){
    curl_easy_setopt(curlHandle.get(), CURLOPT_USERAGENT, userAgent.c_str());
    return *this;
}

Request& Request::addFormField(const std::string& fieldName, const std::string & value){
    if(!mime){
        mime.reset(curl_mime_init(curlHandle.get()));
        if(!mime) throw MimeException("Failed to initialize MIME");
        curl_easy_setopt(curlHandle.get(), CURLOPT_MIMEPOST, mime.get());
    }
    curl_mimepart* part = curl_mime_addpart(mime.get());
    if(!part) throw MimeException("Failed to add MIME part");
    curl_mime_name(part, fieldName.c_str());
    curl_mime_data(part, value.c_str(), CURL_ZERO_TERMINATED);
    return *this;
}

Request& Request::addFormFile(const std::string& fieldName, const std::string & filePath){
    if(!mime){
        mime.reset(curl_mime_init(curlHandle.get()));
        if(!mime) throw MimeException("Failed to initialize MIME");
        curl_easy_setopt(curlHandle.get(), CURLOPT_MIMEPOST, mime.get());
    }
    curl_mimepart* part = curl_mime_addpart(mime.get());
    if(!part) throw MimeException("Failed to add MIME part");
    curl_mime_name(part, fieldName.c_str());
    curl_mime_filedata(part, filePath.c_str());
    return *this;
}

Request& Request::enableVerbose(bool enabled){
    curl_easy_setopt(curlHandle.get(), CURLOPT_VERBOSE, enabled ? 1L : 0L);
    return *this;
}

} // namespace curling
