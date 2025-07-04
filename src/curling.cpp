// Copyright (c) 2025 Paul Caron
// Licensed under the MIT License.
// See LICENSE file in the root of the repository.

#include "curling.hpp"

namespace curling {

Request::Request() : method(Method::GET), curlHandle(nullptr), list(nullptr), cookieFile(""), cookieJar("") {
    detail::ensureCurlGlobalInit();

    curlHandle.reset(curl_easy_init());
    if (!curlHandle) {
        throw InitializationException("Curl initialization failed");
    }

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
    detail::maybeCleanupGlobalCurl();
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
        case Method::HEAD:
            curl_easy_setopt(curlHandle.get(), CURLOPT_NOBODY, 1L);
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

Request& Request::setProgressCallback(ProgressCallback cb){
    progressCallback = cb;
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
        curl_easy_setopt(curlHandle.get(), CURLOPT_COPYPOSTFIELDS, this->body.c_str());
    }
    return *this;
}

Response Request::send(unsigned attempts) {
    if (attempts == 0) {
        throw LogicException("Number of attempts must be greater than zero");
    }

    const unsigned baseDelayMs = 1000; // initial delay of 1 second

    Response response;
    FilePtr fileOut(nullptr);
    std::ostringstream responseStream;

        
    prepareCurlOptions(response, fileOut, responseStream);
    updateURL();
    setCurlHttpVersion();

    for (unsigned attempt = 1; attempt <= attempts; ++attempt) {
        
        try{
            // Perform request
            CURLcode res = curl_easy_perform(curlHandle.get());

            // Get HTTP status code regardless of result
            curl_easy_getinfo(curlHandle.get(), CURLINFO_RESPONSE_CODE, &(response.httpCode));

            if (res != CURLE_OK) {
                throw RequestException(
                    std::string("Curl perform failed on attempt ") + std::to_string(attempt) +
                    ": " + curl_easy_strerror(res)
                );
            }

            // Store response body if not downloading to file
            if (downloadFilePath.empty()) {
                response.body = responseStream.str();
            }

            reset(); // Reset for reuse
            return response;

        } catch (const RequestException& e) {
            if (attempt == attempts) {
                reset();
                throw; // rethrow if final attempt fails
            }

            // Calculate exponential backoff delay
            unsigned delayMs = baseDelayMs * (1 << (attempt - 1));

            // Optional: Add jitter (randomize slightly to avoid thundering herd)
            // delayMs += rand() % 250;

            std::cerr << "Retry attempt " << attempt << " failed. Retrying in " << delayMs << "ms...\n";

            waitMs(delayMs);
        }
    }

    // Should never be reached
    throw LogicException("Retry logic terminated unexpectedly");
}

void Request::reset() {
    // Create and immediately assign new handle
    curlHandle.reset(curl_easy_init());
    if (!curlHandle) {
        throw InitializationException("Curl re-initialization failed");
    }

    mime.reset();
    list.reset();

    args.clear();
    url.clear();
    body.clear();
    downloadFilePath.clear();
    progressCallback = nullptr;
    cookieFile.clear();
    cookieJar.clear();

    method = Method::GET;
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTPGET, 1L);

    httpVersion = HttpVersion::DEFAULT;
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
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
    //set member variables
    cookieFile = path;
    cookieJar = path;
    //set path to read cookies from
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEFILE, path.c_str());
    //set path to write cookies to
    curl_easy_setopt(curlHandle.get(), CURLOPT_COOKIEJAR, path.c_str());
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


Request& Request::setHttpVersion(HttpVersion version) {
    
    curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);

    switch (version) {
        case HttpVersion::HTTP_2:
            if (!(info->features & CURL_VERSION_HTTP2)) {
                throw LogicException("HTTP/2 is not supported by the current libcurl build.");
            }
            break;
        case HttpVersion::HTTP_3:
            if (!(info->features & CURL_VERSION_HTTP3)) {
                throw LogicException("HTTP/3 is not supported by the current libcurl build.");
            }
            break;
        default:
            break; // No check needed for DEFAULT or HTTP_1_1
    }

    this->httpVersion = version;
    return *this;
}

void Request::prepareCurlOptions(Response& response, FilePtr& fileOut, std::ostringstream& responseStream) {
    // Set progress callback if defined
    if (progressCallback) {
        curl_easy_setopt(curlHandle.get(), CURLOPT_XFERINFOFUNCTION, detail::ProgressCallbackBridge);
        curl_easy_setopt(curlHandle.get(), CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curlHandle.get(), CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(curlHandle.get(), CURLOPT_NOPROGRESS, 1L);
    }

    // Set output destination (file or memory stream)
    if (!downloadFilePath.empty()) {
        fileOut.reset(std::fopen(downloadFilePath.c_str(), "wb"));
        if (!fileOut) {
            throw RequestException("Failed to open file for writing: " + downloadFilePath);
        }
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEFUNCTION, nullptr);
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, fileOut.get());
    } else {
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEFUNCTION, detail::WriteCallback);
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, &responseStream);
    }

    // Set header callback
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERDATA, &(response.headers));
}

void Request::setCurlHttpVersion() {
    long curl_http_version = CURL_HTTP_VERSION_NONE;
    switch (httpVersion) {
        case HttpVersion::HTTP_1_1: curl_http_version = CURL_HTTP_VERSION_1_1; break;
        case HttpVersion::HTTP_2:   curl_http_version = CURL_HTTP_VERSION_2_0; break;
        case HttpVersion::HTTP_3:   curl_http_version = CURL_HTTP_VERSION_3;   break;
        case HttpVersion::DEFAULT:
        default:                    curl_http_version = CURL_HTTP_VERSION_NONE; break;
    }
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTP_VERSION, curl_http_version);
}

} // namespace curling
