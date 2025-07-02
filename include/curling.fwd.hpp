#pragma once

// Forward declarations of libcurl types (global namespace)
struct CURL;
struct curl_slist;
struct curl_mime;

namespace curling {

// --- Exception classes ---
class CurlingException;
class InitializationException;
class RequestException;
class HeaderException;
class MimeException;
class LogicException;

// --- Core types ---
class Request;
struct Response;

// --- Smart pointer deleters ---
struct CurlHandleDeleter;
struct CurlSlistDeleter;
struct CurlMimeDeleter;

// --- Smart pointer aliases ---
template<typename T>
using unique_curl_ptr = std::unique_ptr<T>;

using CurlPtr = unique_curl_ptr<CURL>;
using CurlSlistPtr = unique_curl_ptr<curl_slist>;
using CurlMimePtr = unique_curl_ptr<curl_mime>;

// --- detail namespace (internal utilities) ---
namespace detail {
    int ProgressCallbackBridge(void* clientp,
                               curl_off_t dltotal, curl_off_t dlnow,
                               curl_off_t ultotal, curl_off_t ulnow);
}

} // namespace curling
