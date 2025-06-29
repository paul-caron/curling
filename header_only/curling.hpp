/*
 * Copyright (c) 2025 Paul Caron
 *
 * This file is part of Curling - a modern C++ wrapper for libcurl.
 *
 * Licensed under the MIT License. You may obtain a copy of the license at
 * https://opensource.org/licenses/MIT
 */


/**
 * @mainpage Curling: Modern C++ libcurl Wrapper
 *
 * Curling is a lightweight, header-only C++17 wrapper around libcurl for
 * making HTTP requests with a modern design and safe resource handling.
 *
 * @section features Features
 * - RAII and smart-pointer-based resource management
 * - MIME support for file uploads
 * - Fluent API for intuitive chaining
 * - Proxy and authentication support
 * - Persistent cookie management
 *
 * @section example Example
 * @code
 * curling::Request req;
 * req.setMethod(curling::Request::Method::POST)
 *    .setURL("https://example.com")
 *    .addHeader("Content-Type: application/json")
 *    .setBody(R"({"key": "value"})");
 * curling::Response res = req.send();
 * std::cout << res.toString();
 * @endcode
 */

/**
 * @note If you use Method::MIME (multipart POST), you must reset the Request
 * before switching to another method. Attempting to change it afterward throws logic_error.
 */


/**
 * @note Curling internally manages curl_global_init() and curl_global_cleanup()
 * using std::once_flag. You don’t need to do this manually.
 */

/**
 * @note Header keys in Response::headers are stored in lowercase
 * to support case-insensitive lookup.
 */

/**
 * @note By default, cookies are persisted in "cookies.txt". Override this via setCookiePath().
 */

/**
 * @note Calling enableVerbose(true) enables libcurl's verbose output to stderr,
 * which is useful for debugging.
 */

/**
 * @warning This class is not thread-safe. Do not share a Request instance across threads.
 * Each thread should use its own Request object.
 */

#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <memory>
#include <functional>
#include <curl/curl.h>

namespace curling {

inline constexpr int version_major = 1;
inline constexpr int version_minor = 1;
inline constexpr int version_patch = 0;

std::string version();

/**
 * @brief Util/helper section
 * @note meant for internal library use only
 */
namespace detail {

inline void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){
        return !std::isspace(ch);
    }).base(), s.end());
}

inline void toLowerCase(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

int ProgressCallbackBridge(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                  curl_off_t ultotal, curl_off_t ulnow);


}//detail end

/**
 * @class CurlingException
 * @brief Base exception class for Curling errors.
 */
class CurlingException : public std::runtime_error {
public:
    explicit CurlingException(const std::string& msg) : std::runtime_error(msg) {}
};

/** @class InitializationException
 * @brief Thrown when curl initialization fails.
 */
class InitializationException : public CurlingException {
public:
    explicit InitializationException(const std::string& msg) : CurlingException(msg) {}
};

/** @class RequestException
 * @brief Thrown when a request operation fails.
 */
class RequestException : public CurlingException {
public:
    explicit RequestException(const std::string& msg) : CurlingException(msg) {}
};

/** @class HeaderException
 * @brief Thrown when header operations fail.
 */
class HeaderException : public CurlingException {
public:
    explicit HeaderException(const std::string& msg) : CurlingException(msg) {}
};

/** @class MimeException
 * @brief Thrown when MIME operations fail.
 */
class MimeException : public CurlingException {
public:
    explicit MimeException(const std::string& msg) : CurlingException(msg) {}
};

/** @class LogicException
 * @brief Thrown when library logic prohibits an operation.
 */
class LogicException : public CurlingException {
public:
    explicit LogicException(const std::string& msg) : CurlingException(msg) {}
};

/** SAFETY: RAII deleters for CURL handles */
struct CurlHandleDeleter { void operator()(CURL* h) const noexcept { if (h) curl_easy_cleanup(h); }};
struct CurlSlistDeleter { void operator()(curl_slist* l) const noexcept { if (l) curl_slist_free_all(l); }};
struct CurlMimeDeleter { void operator()(curl_mime* m) const noexcept { if (m) curl_mime_free(m); }};

using CurlPtr = std::unique_ptr<CURL, CurlHandleDeleter>;
using CurlSlistPtr = std::unique_ptr<curl_slist, CurlSlistDeleter>;
using CurlMimePtr = std::unique_ptr<curl_mime, CurlMimeDeleter>;

/**
 * @struct Response
 * @brief Represents an HTTP response.
 *
 * Contains the HTTP status code, body, and headers.
 */
struct Response {
    long httpCode; ///< HTTP status code.
    std::string body; ///< Response body.
    std::map<std::string, std::vector<std::string>> headers; ///< Header map (key: lowercase).
    
    std::string toString() const {
        std::ostringstream oss;
        oss << "status: " << httpCode << "\nbody:\n" << body << "\nheaders:\n";
        for (auto const& h : headers) {
            oss << h.first << ": ";
            for (auto const& v : h.second) oss << v << " ";
            oss << "\n";
        }
        return oss.str();
    }
    std::vector<std::string> getHeader(const std::string& key) const {
        std::string lowered = key;
        detail::toLowerCase(lowered);
        auto it = headers.find(lowered);
        return (it != headers.end()) ? it->second : std::vector<std::string>{};
    }
};

/**
 * @class Request
 * @brief Provides a fluent wrapper for HTTP requests via libcurl.
 */
class Request {
public:
    using ProgressCallback = std::function<bool(curl_off_t dltotal, curl_off_t dlnow,
                                                curl_off_t ultotal, curl_off_t ulnow)>;

    /**
     * @enum Method
     * @brief Supported HTTP methods.
     */
    enum class Method {
        GET,    ///< Standard GET
        POST,   ///< Standard POST
        PUT,    ///< PUT
        DEL,    ///< DELETE (named DEL to avoid macro clash)
        PATCH,  ///< PATCH
        HEAD,   ///< HEAD request just headers
        MIME    ///< Multipart/form-data POST
    };

    /**
     * @enum AuthMethod
     * @brief HTTP authentication schemes.
     */
    enum class AuthMethod {
        BASIC = CURLAUTH_BASIC,
        NTLM = CURLAUTH_NTLM,
        DIGEST = CURLAUTH_DIGEST
    };

    /**
    * @enum HttpVersion
    * @brief Specifies the HTTP version to be used for the request.
    *
    * Allows explicit control over the HTTP version that libcurl should use
    * when performing a request. If the selected version is not supported
    * by the libcurl build or the server, fallback behavior may occur.
    *
    * @note If you request a version not supported by libcurl at runtime,
    * Curling will throw a LogicException during configuration.
    * If the server doesn't support the version (e.g., HTTP/2),
    * libcurl may fall back to an older version without error.
    */
    enum class HttpVersion {
        DEFAULT,   ///< Let libcurl automatically negotiate the best supported HTTP version.
        HTTP_1_1,  ///< Force HTTP/1.1 for all requests.
        HTTP_2,    ///< Force HTTP/2 (requires libcurl built with nghttp2 support).
        HTTP_3     ///< Force HTTP/3 (requires libcurl built with HTTP/3 support, e.g. with quiche or ngtcp2).
    };

    /**
     * @brief Constructor initializes curl global state.
     * @throws InitializationException if initialization fails.
     */
    Request();

    /**
     * @brief Destructor cleans up curl state if last instance.
     */
    ~Request() noexcept;

    Request(Request&&) noexcept;
    Request& operator=(Request&&) noexcept;

    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;

    /**
     * @brief Sets the progress callback function.
     * @param cb Callback receiving download/upload progress. Return true to abort.
     * @return *this
     */
    Request& setProgressCallback(ProgressCallback cb);

    /**
     * @brief Sets the HTTP method for the request.
     * @param m Enum value for HTTP method.
     * @return *this
     */
    Request& setMethod(Method m);

    /**
     * @brief Sets the request URL.
     * @param url URL to fetch.
     * @return *this
     */
    Request& setURL(const std::string& url);

    /**
     * @brief Enables proxy usage.
     * @param url Proxy URL.
     * @return *this
     */
    Request& setProxy(const std::string& url);

    /**
     * @brief Sets credentials for proxy authentication.
     * @param username Proxy username.
     * @param password Proxy password.
     * @return *this
     */
    Request& setProxyAuth(const std::string& username, const std::string& password);

    /**
     * @brief Sets proxy authentication scheme.
     * @param method Authentication method.
     * @return *this
     */
    Request& setProxyAuthMethod(AuthMethod method);

    /**
     * @brief Sets credentials for HTTP auth (Basic/Digest/NTLM).
     * @param username Username.
     * @param password Password.
     * @return *this
     */
    Request& setHttpAuth(const std::string& username, const std::string& password);

    /**
     * @brief Sets HTTP authentication scheme.
     * @param method Authentication method.
     * @return *this
     */
    Request& setHttpAuthMethod(AuthMethod method);

    /**
     * @brief Adds a query parameter to the URL.
     * @param key Parameter name.
     * @param value Parameter value.
     * @return *this
     */
    Request& addArg(const std::string& key, const std::string& value);

    /**
     * @brief Adds a custom HTTP header.
     * @param header A full header line, e.g. "Accept: application/json".
     * @return *this
     */
    Request& addHeader(const std::string& header);

    /**
     * @brief Sets the body of the request (for POST/PUT/PATCH).
     * @param body Request body content.
     * @return *this
     */
    Request& setBody(const std::string& body);

    /**
     * @brief Enables download streaming to a file.
     * @param path Local file path for saving response.
     * @return *this
     */
    Request& downloadToFile(const std::string& path);

    /**
     * @brief Sets a timeout for the request (in seconds).
     * @param seconds Timeout in seconds.
     * @return *this
     */
    Request& setTimeout(long seconds);

    /**
     * @brief Sets connection timeout (in seconds).
     * @param seconds Timeout in seconds.
     * @return *this
     */
    Request& setConnectTimeout(long seconds);

    /**
     * @brief Enables or disables automatic redirect-following.
     * @param follow True to follow redirects.
     * @return *this
     */
    Request& setFollowRedirects(bool follow);

    /**
     * @brief Adds a Bearer token for Authorization header.
     * @param token Bearer token string.
     * @return *this
     */
    Request& setAuthToken(const std::string& token);

    /**
     * @brief Overrides default cookie file for persistence.
     * @param path File path for storing cookies.
     * @return *this
     */
    Request& setCookiePath(const std::string& path);

    /**
     * @brief Sets the User-Agent header.
     * @param userAgent Agent string.
     * @return *this
     */
    Request& setUserAgent(const std::string& userAgent);

    /**
     * @brief Adds a field to multipart/form-data.
     * @param fieldName Field name.
     * @param value Field value.
     * @return *this
     * @throws MimeException on internal curl errors.
     */
    Request& addFormField(const std::string& fieldName, const std::string& value);

    /**
     * @brief Adds a file to multipart upload.
     * @param fieldName Field name.
     * @param filePath Path to file on disk.
     * @return *this
     * @throws MimeException on internal curl errors.
     */
    Request& addFormFile(const std::string& fieldName, const std::string& filePath);

    /**
     * @brief Enables or disables libcurl verbose output.
     * @param enabled True to enable verbose mode.
     * @return *this
     */
    Request& enableVerbose(bool enabled = true);

    /**
     * @brief Executes the HTTP request.
     * @return Response object with status, body, headers.
     * @throws RequestException on failure.
     */
    Response send();

    /**
     * @brief Resets internal state to allow reuse.
     */
    void reset();

    /**
     * @brief Set the HTTP protocol version (http1.1, 2 or 3)
     */
    Request& setHttpVersion(HttpVersion version);

    /**
     * @brief Low level access to define curl options
     *
     * @warning Should be used with caution. Meant for the libcurl advanced users.
     */
    template<typename T>
    Request& setRawOption(CURLoption opt, T value) {
        static_assert(std::is_pointer<T>::value || std::is_arithmetic<T>::value,
                      "setRawOption only supports pointer or arithmetic types");
        curl_easy_setopt(curlHandle.get(), opt, value);
        return *this;
    }

    friend int detail::ProgressCallbackBridge(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                          curl_off_t ultotal, curl_off_t ulnow);


private:
    Method method;
    CurlPtr curlHandle;
    CurlSlistPtr list;//headers;
    std::string url, args, body, cookieFile, cookieJar;
    CurlMimePtr mime;
    std::string downloadFilePath;
    ProgressCallback progressCallback;
    HttpVersion httpVersion = HttpVersion::DEFAULT;

    void clean() noexcept;
    void updateURL();
};

static_assert(!std::is_copy_constructible_v<Request> && !std::is_copy_assignable_v<Request>,
              "curling::Request is not copyable: it is thread-unsafe and must not be shared between threads. One instance per thread.");

} // namespace curling


namespace curling {

namespace detail{

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

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto responseStream = static_cast<std::ostringstream*>(userp);
    responseStream->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto* headerMap = static_cast<std::map<std::string, std::vector<std::string>>*>(userdata);
    std::string headerLine(buffer, size * nitems);

    if (headerLine.empty()) return 0; // skip the separation line

    auto colonPos = headerLine.find(":");
    if (colonPos != std::string::npos) {
        std::string key = headerLine.substr(0, colonPos);
        std::string value = headerLine.substr(colonPos + 1);
        detail::trim(key);
        detail::trim(value);
        detail::toLowerCase(key);
        (*headerMap)[key].push_back(value);
    }

    return size * nitems;
}

int ProgressCallbackBridge(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                    curl_off_t ultotal, curl_off_t ulnow) {
    auto* req = static_cast<Request*>(clientp);
    if (req->progressCallback) {
        bool shouldCancel = req->progressCallback(dltotal, dlnow, ultotal, ulnow);
        return shouldCancel ? 1 : 0; // Returning non-zero aborts transfer
    }
    return 0;
}

}//end of detail namespace

std::string version() {
    std::ostringstream oss;
    oss << version_major << '.' << version_minor << '.' << version_patch;
    return oss.str();
}

Request::Request() : method(Method::GET), curlHandle(nullptr), list(nullptr), cookieFile("cookies.txt"), cookieJar("cookies.txt") {
    detail::ensureCurlGlobalInit();
    
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
            curl_easy_setopt(curlHandle.get(), CURLOPT_HEADER, 1L);
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
        curl_easy_setopt(curlHandle.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(this->body.size()));
        curl_easy_setopt(curlHandle.get(), CURLOPT_COPYPOSTFIELDS, this->body.data());
    }
    return *this;
}

Response Request::send() {
    Response response;
    FILE* fileOut = nullptr;
    std::ostringstream responseStream;

    // Set progress callback if present
    if (progressCallback) {
        curl_easy_setopt(curlHandle.get(), CURLOPT_XFERINFOFUNCTION, detail::ProgressCallbackBridge);
        curl_easy_setopt(curlHandle.get(), CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curlHandle.get(), CURLOPT_NOPROGRESS, 0L);
    }

    // Set write target (file or memory)
    if (!downloadFilePath.empty()) {
        fileOut = std::fopen(downloadFilePath.c_str(), "wb");
        if (!fileOut) {
            throw RequestException("Failed to open file for writing: " + downloadFilePath);
        }
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, fileOut);
    } else {
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEFUNCTION, detail::WriteCallback);
        curl_easy_setopt(curlHandle.get(), CURLOPT_WRITEDATA, &responseStream);
    }

    // Set headers callback
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERFUNCTION, detail::HeaderCallback);
    curl_easy_setopt(curlHandle.get(), CURLOPT_HEADERDATA, &(response.headers));

    updateURL();

    // Set HTTP version
    long curl_http_version = CURL_HTTP_VERSION_NONE;
    switch (httpVersion) {
        case HttpVersion::HTTP_1_1: curl_http_version = CURL_HTTP_VERSION_1_1; break;
        case HttpVersion::HTTP_2:   curl_http_version = CURL_HTTP_VERSION_2_0; break;
        case HttpVersion::HTTP_3:   curl_http_version = CURL_HTTP_VERSION_3;   break;
        case HttpVersion::DEFAULT:
        default:                    curl_http_version = CURL_HTTP_VERSION_NONE; break;
    }
    curl_easy_setopt(curlHandle.get(), CURLOPT_HTTP_VERSION, curl_http_version);

    // Perform request
    CURLcode res = curl_easy_perform(curlHandle.get());

    // Get HTTP status code regardless of result
    curl_easy_getinfo(curlHandle.get(), CURLINFO_RESPONSE_CODE, &(response.httpCode));

    // Close file if it was opened
    if (fileOut) {
        std::fclose(fileOut);
        fileOut = nullptr;
    }

    // Handle errors
    if (res != CURLE_OK) {
        char* effectiveUrl = nullptr;
        curl_easy_getinfo(curlHandle.get(), CURLINFO_EFFECTIVE_URL, &effectiveUrl);

        std::ostringstream err;
        err << "Curl perform failed for URL: " 
            << (effectiveUrl ? effectiveUrl : (url + (args.empty() ? "" : "?" + args)))
            << "\nError Code: " << res << " (" << curl_easy_strerror(res) << ")"
            << "\nHTTP Status Code: " << response.httpCode;

        throw RequestException(err.str());
    }

    // Store response body if not downloaded to file
    if (downloadFilePath.empty()) {
        response.body = responseStream.str();
    }

    reset(); // Reset state for reuse

    return response;
}

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

} // namespace curling
