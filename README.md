# curling
A C++ libcurl wrapper

## compile
```sh
git clone https://github.com/paul-caron/curling
cd curling
make
```

## dependencies
generally speaking you need these packages, the names might differ on your distro
```
sudo apt install curl libcurl libcurl-dev
```
on my ubuntu 24 machine, i installed the following:
```
sudo apt install curl libcurl3t64-gnutls libcurl4-openssl-dev libcurl4t64
```
also needs some compiling basic tools
```
sudo apt install g++ make
```

## documentation
for generating documentation, install doxygen and graphviz and then run:
```
make doc
```
or you can take a look into the precompiled pdf manual included in this repo: refman.pdf

### Key Classes and Structures

1. **`Response` Class:**
   - Stores the results from an HTTP request, including:
     - `httpCode`: The HTTP status code returned by the server.
     - `body`: The body content of the response.
     - `headers`: A map to store headers with multiple values for the same 
header key.

2. **`Request` Class:**
   - Manages the construction and sending of HTTP requests using libcurl.
   - Provides methods to set various options like HTTP method, URL, 
headers, body content, proxy settings, authentication, cookies, etc.

### Key Methods in `Request`

- **`setMethod(Method m)`**: Sets the HTTP method (GET, POST, PUT, DELETE, 
PATCH) and configures libcurl accordingly.
- **`setURL(const std::string& URL)`**: Sets the request's target URL.
- **`addArg(const std::string& key, const std::string& value)`**: Appends 
query parameters to the URL for GET requests.
- **`addHeader(const std::string& header)`**: Adds an HTTP header to the 
request.
- **`setBody(const std::string& body)`**: Sets the body content for 
methods that support it (POST, PUT, PATCH).
- **`send()`**: Executes the request and retrieves the response. It 
captures both headers and body using libcurl's callback mechanism.
- **`reset()`**: Resets the request object to its initial state, preparing 
it for reuse.
- **`clean()`**: Cleans up any allocated resources, such as curl handles 
and lists.

### Callback Functions

1. **`WriteCallback`**:
   - Writes response data into a `std::ostringstream`, which allows easy 
retrieval of the response body after the request completes.

2. **`HeaderCallback`**:
   - Processes HTTP headers line by line and stores them in a map, 
handling cases where multiple values exist for a single header key.

### Additional Functionalities

- **Timeouts**: Methods to set connection and overall request timeouts.
- **Proxy Settings**: Configure proxy URL and authentication.
- **Cookie Handling**: Set paths to read/write cookies.
- **User-Agent**: Customize the user-agent string in requests.
- **Multipart/form-data**: Support for adding form fields and file 
uploads.

### Usage

To use this `Request` class, you would typically:
1. Create an instance of `Request`.
2. Use various setter methods (`setMethod`, `setURL`, etc.) to configure 
the request.
3. Call `send()` to execute the request and receive a `Response` object 
containing the results.

This design provides a structured way to interact with HTTP services in 
C++ applications, leveraging libcurl for network operations while 
providing a user-friendly interface for developers.
