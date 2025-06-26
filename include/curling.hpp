#ifndef CURLING_HPP
#define CURLING_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <locale>
#include <memory>
#include <curl/curl.h>

namespace curling {

struct CurlHandleDeleter {
    void operator()(CURL * handle) const{
        if (handle) curl_easy_cleanup(handle);
    }
};

struct CurlSlistDeleter {
    void operator()(curl_slist * list) const{
        if(list) curl_slist_free_all(list);
    }
};

struct CurlMimeDeleter {
    void operator()(curl_mime* mime) const {
        if(mime) curl_mime_free(mime);
    }
};

using CurlPtr = std::unique_ptr<CURL, CurlHandleDeleter>;
using CurlSlistPtr = std::unique_ptr<curl_slist, CurlSlistDeleter>;
using CurlMimePtr = std::unique_ptr<curl_mime, CurlMimeDeleter>;

/**
 * @struct Response
 * @brief Represents an HTTP response.
 *
 * This structure holds details of an HTTP response, including 
 * the status code, body content, and headers.
 */
struct Response {
    long httpCode; ///< The HTTP status code received in the response.

    std::string body; ///< The body content of the HTTP response as a string.

    /**
     * @brief A map to store HTTP headers from the response.
     *
     * Each header is stored with its name as the key and 
     * the corresponding value is stored into a vector, as there can be many headers with same key.
     */
    std::map<std::string, std::vector<std::string>> headers;
    std::string toString() const {
        std::ostringstream oss;
        oss << "status: " << httpCode << "\nbody: \n" << body << "\nheaders: \n";
        for(const auto& h: headers){
            oss << h.first << ": ";
            for(const auto& value: h.second){
                oss << value << " ";
            }
            oss << std::endl;
        }
        return oss.str();
    }
};


/**
 * @class Request
 * @brief Handles HTTP requests using libcurl.
 *
 * This class provides functionality to perform HTTP operations such as GET, POST,
 * PUT, and DELETE. It manages the setup and execution of these requests with libcurl.
 */
class Request {
public:
    /**
     * @enum Method
     * @brief Enumerates the supported HTTP methods.
     */
    enum class Method { 
        GET, ///< Represents an HTTP GET request.
        POST, ///< Represents an HTTP POST request.
        PUT, ///< Represents an HTTP PUT request.
        DEL, ///< Represents an HTTP DELETE request.
        PATCH, ///< Represents an HTTP PATCH request.
        MIME, ///< Represents an HTTP POST request. A pseudo method MIME for multipart forms (eg.. file upload)
    };

    /**
     * @brief Constructor for the Request class.
     *
     * Initializes a new instance of the Request class and increments the 
     * instances counter.
     */
    Request();

    /**
     * @brief Destructor for the Request class.
     *
     * Decrements the instances counter and performs any necessary cleanup.
     */
    ~Request();

    // move constructors
    Request(Request&& other) noexcept;
    Request& operator=(Request&&) noexcept;

    // deleted copy constructors
    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;


    /**
     * @brief Sets the HTTP method for the request.
     *
     * @param m The method to set (e.g., Method::GET, Method::POST).
     */
    Request& setMethod(Method m);

    /**
     * @brief Sets the URL for the request.
     *
     * @param URL The URL string to be used in the HTTP request.
     */
    Request& setURL(const std::string& URL);

    /**
     * @brief Sets the URL for the request.
     *
     * @param URL The URL of the Proxy, a string.
     */
    Request& setProxy(const std::string& URL);

    /**
     * @brief Sets the auth credentials for the proxy
     *
     * @param username
     * @param password
     */
    Request& setProxyAuth(const std::string& username, const std::string & password);


    /**
     * @brief Sets the Auth Method used with proxies
     *
     * @param method
     */
    Request& setProxyAuthMethod(long method);

    /**
     * @brief Adds an argument to the query string of the request.
     *
     * @param key The arg key
     *
     * @param value The arg value
     */
    Request& addArg(const std::string& key, const std::string& value);

    /**
     * @brief Adds a header to the request.
     *
     * @param header The header string to be added (e.g., "Content-Type: text/html").
     */
    Request& addHeader(const std::string& header);

    /**
     * @brief Sets the body of the HTTP request, applicable for POST and PUT methods.
     *
     * @param body The body content as a string.
     */
    Request& setBody(const std::string& body);

    /**
     * @brief Sends the HTTP request using libcurl.
     */
    Response send();

    /**
     * @brief Resets the internal state for reuse of this Request instance.
     */
    void reset();


    /**
     * @brief Sets timeout of request
     *
     * This method is to limit the amount of time per request
     */
    Request& setTimeout(long seconds);

    /**
     * @brief Sets follow redirects
     *
     * This method is to enable or disable follow redirects.
     */
    Request& setFollowRedirects(bool follow);


    /**
     * @brief Sets timeout to connect
     *
     * This method is to limit the amount of time per connection
     */
    Request& setConnectTimeout(long seconds);

    /**
     * @brief Sets the header "Autorization: Bearer <token>"
     *
     * This method sets the header with a given authorization token.
     */
    Request& setAuthToken(const std::string& token);

    /**
     * @brief Sets the cookie file path
     *
     * This method sets the name or path of the file that curl will write cookies into. Default "cookies.txt".
     */
    Request& setCookiePath(const std::string& path);


    /**
     * @brief Sets the User Agent header
     *
     * This method sets the user agent header that will be sent with the request
     */
    Request& setUserAgent(const std::string& userAgent);


    /**
     * @brief Adds a form field
     *
     * This method adds a form field for a multipart form post request
     */
    Request& addFormField(const std::string& fieldName, const std::string & value);

    /**
     * @brief Adds a form file
     *
     * This method add a file to upload during the multipart post request
     */
    Request& addFormFile(const std::string& fieldName, const std::string & filePath);


    /**
     * @brief Enable or disable curl's verbose mode
     *
     */
    Request& enableVerbose(bool enabled = true);

private:
    Method method;
    CurlPtr curlHandle;
    CurlSlistPtr list;
    std::string url, args, body, cookieFile, cookieJar;
    CurlMimePtr mime = nullptr;

    /**
     * @brief Callback function for handling the data received in the response body.
     *
     * This static function is used as a callback to handle incoming data when
     * writing the response body during a curl operation.
     *
     * @param contents Pointer to the data buffer.
     * @param size Size of each element.
     * @param nmemb Number of elements in the buffer.
     * @param userp User pointer, typically pointing to an instance of Request.
     * @return The number of bytes actually taken care of by this function,
     *         which may differ from the size*nmemb if the data is not
     *         completely processed here (due to a call to Curl_readdata()
     *         for example).
     */
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    /**
     * @brief Callback function for handling response headers.
     *
     * This static function is used as a callback to handle incoming header data
     * during a curl operation. It processes and stores the headers in a map.
     *
     * @param buffer The header line being processed.
     * @param size Size of each character.
     * @param nitems Number of items (characters) in the buffer.
     * @param userdata User pointer, typically pointing to an instance of Request.
     * @return The number of bytes actually taken care of by this function,
     *         which may differ from size*nitems if the data is not completely
     *         processed here.
     */
    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);

    /**
     * @brief Cleans up resources associated with the current request.
     *
     * This private method ensures that all libcurl and other allocated resources
     * are properly freed and reset.
     */
    void clean();

    /**
     * @brief Updates the URL to include query arguments if present.
     *
     * This method appends the stored query arguments to the base URL if any 
     * arguments have been added using addArg().
     */
    void updateURL();

    /**
     * @brief Trims whitespaces from string
     *
     * This method is used to trim spaces during parsing of the response headers
     */
    static void trim(std::string & s);

};

} // namespace curling

#endif // CURLING_HPP
