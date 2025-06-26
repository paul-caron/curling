What is Curling?

> A modern C++ wrapper for libcurl supporting HTTP/HTTPS requests with JSON, file uploads, proxy, cookies, and more.



Features:

Fluent API (chainable)

Multipart/MIME support (e.g. file uploads)

Cookie management

Proxy and Auth support

Supports GET, POST, PUT, DELETE, PATCH, and multipart

Linux .deb package support


Installation:

With Make

With .deb

Suggested future: with CMake, vcpkg, or Conan


Basic Usage:
```
curling::Request req;
auto res = req.setMethod(Request::Method::GET)
              .setURL("https://example.com")
              .send();

std::cout << res.toString();
```
