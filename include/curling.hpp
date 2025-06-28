#pragma once

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
 * @warning This class is not thread-safe. Do not share a Request instance across threads.
 * Each thread should use its own Request object.
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
inline constexpr int version_minor = 0;
inline constexpr int version_patch = 0;

std::string version();

// utils
namespace detail {

inline size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
inline size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
inline int ProgressCallbackBridge(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                  curl_off_t ultotal, curl_off_t ulnow);

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

    void clean() noexcept;
    void updateURL();
};



} // namespace curling

