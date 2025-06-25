#ifndef CURLING_HPP
#define CURLING_HPP

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <atomic>
#include <curl/curl.h>

namespace curling {

struct Response {
    long httpCode;
    std::string body;
    std::map<std::string, std::string> headers;
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
        DELETE ///< Represents an HTTP DELETE request.
    };

    /**
     * @var instances
     * @brief Static atomic counter to track the number of Request instances.
     */
    inline static std::atomic<int> instances = 0;

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

    /**
     * @brief Sets the HTTP method for the request.
     *
     * @param m The method to set (e.g., Method::GET, Method::POST).
     */
    void setMethod(Method m);

    /**
     * @brief Sets the URL for the request.
     *
     * @param URL The URL string to be used in the HTTP request.
     */
    void setURL(const std::string& URL);

    /**
     * @brief Adds an argument to the query string of the request.
     *
     * @param arg The key-value pair formatted as a string (e.g., "key=value").
     */
    void addArg(const std::string& arg);

    /**
     * @brief Adds a header to the request.
     *
     * @param header The header string to be added (e.g., "Content-Type: text/html").
     */
    void addHeader(const std::string& header);

    /**
     * @brief Sets the body of the HTTP request, applicable for POST and PUT methods.
     *
     * @param body The body content as a string.
     */
    void setBody(const std::string& body);

    /**
     * @brief Sends the HTTP request using libcurl.
     */
    Response send();

    /**
     * @brief Retrieves the response body from the last sent request.
     *
     * @return A constant reference to the response string.
     */
    const std::string& getResponse() const;

    /**
     * @brief Retrieves a map of response headers from the last sent request.
     *
     * @return A constant reference to the map containing response headers.
     */
    const std::map<std::string, std::string>& getResponseHeadersMap() const;

    /**
     * @brief Retrieves http code of the response from the last sent request.
     *
     * @return A constant reference to the map containing response headers
     */
    const long& getHttpCode() const;

    /**
     * @brief Resets the internal state for reuse of this Request instance.
     */
    void reset();

private:
    Method method;
    CURL* curlHandle;
    struct curl_slist* list;
    std::string url, args, response, body, cookieFile, cookieJar;
    std::map<std::string, std::string> responseHeadersMap;
    long http_code;

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
};

} // namespace curling

#endif // CURLING_HPP
