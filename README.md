![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg) 
![Tests](https://img.shields.io/badge/tests-passing-brightgreen) 
[![Build and Test](https://github.com/paul-caron/curling/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/paul-caron/curling/actions/workflows/build-and-test.yml) 
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) 
![GitHub release](https://img.shields.io/github/v/release/paul-caron/curling?include_prereleases&sort=semver)

# 🌀 Curling

Curling is a modern C++17 wrapper around libcurl, designed to simplify HTTP/HTTPS requests using a clean, fluent API.

It supports JSON payloads, file uploads, cookie management, authentication, proxy configuration, and more — all with a RAII-safe design using smart pointers.


---

## 📚 Table of Contents

✨ Features

🛠 Installation

🚀 Basic Usage

✅ Example Test Case

🧠 Internals & Design

🤔 Why Curling?

⚖️ Comparisons

📚 Documentation

🧪 Testing

🤝 Contributing

📄 License

👤 Maintainer

📌 Notes



---

## ✨ Features

🔁 Fluent API — chainable and expressive

📤 Multipart/MIME support — for file uploads

🍪 Cookie management — with persistent storage

🛡 Proxy and authentication support — including Basic, Bearer, and Digest

🌐 Full HTTP verb support — GET, POST, PUT, DELETE, PATCH, MIME

📦 Linux .deb packaging

🧪 CI-tested with Doctest framework and GitHub Actions



---

## 🛠 Installation

### 🔧 Build shared library

```bash
make
```

To install and package as a Debian .deb:

```bash
make install
make deb
```

Install it locally:

```bash
sudo apt install ./curling_1.0_amd64.deb
sudo ldconfig
```

### 🪶 Header-only version

```bash
make header-only
```

This produces a header-only version in:

`curling/header-only/`


---

## 🚀 Basic Usage

```cpp
#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;

    auto res = req.setMethod(curling::Request::Method::GET)
                  .setURL("https://example.com")
                  .setUserAgent("Lizardzilla/6.9 (Reptilian Humanoid)")
                  .send();

    std::cout << res.toString();
    return 0;
}
```

### 🔨 Compile

With shared library:
```bash
g++ main.cpp -lcurling
```
With header-only:
```bash
g++ main.cpp -lcurl
```

---

## ✅ Example Test Case

Tests run automatically on every push via GitHub Actions.

```cpp
TEST_CASE("GET request to download image") {
    curling::Request req;
    req.setURL("https://httpbin.org/image/png")
       .downloadToFile("out.png");

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(std::filesystem::exists("out.png"));
}
```

Run tests locally:
```bash
make test
```

---

## 🧠 Internals & Design

Curling centers around the curling::Request class, which wraps libcurl functionality in a fluent, type-safe, modern C++ API.

Key principles:

✅ RAII & smart pointers — automatic resource cleanup

✅ Fluent chaining — readable and efficient method calls

✅ No global state — avoids curl_global_* leaks

✅ Safe-by-default — redirects off by default, verbose off, etc.



---

## 🤔 Why Curling?

Libcurl is powerful, but its C API is verbose and error-prone.

Curling offers:

|Feature	|libcurl	|Curling|
|-----------|-----------|-------|
|Fluent C++ API	|❌	|✅|
|RAII memory management	|❌	|✅|
|Built-in test coverage	|❌	|✅|
|Easy file & MIME upload	|Manual	|✅|
|Modern build integration	|❌	|✅|


---

## ⚖️ Comparisons with Other Popular C++ libcurl Wrappers

Curling aims to strike a balance between modern C++ design, fine-grained control, and ease of use. Here's how it compares to other popular libcurl wrappers:

| Aspect              | CPR                           | curlpp                      | Curling (this project)                     |
|---------------------|-------------------------------|-----------------------------|--------------------------------------------|
| API Style           | High-level, intuitive          | Classic wrapper, less modern| Modern C++17 fluent API with chaining      |
| Abstraction Level   | Heavy abstraction, hides internals | Moderate abstraction, outdated | Balanced abstraction, exposes fine control|
| Memory Management   | Manual (raw pointers internally) | Manual                      | RAII with smart pointers                    |
| Feature Completeness| Common HTTP (GET/POST/JSON/auth)| Full libcurl coverage, less ergonomic | Full HTTP verbs, MIME, advanced auth       |
| File/MIME Upload    | Partial support, verbose       | Supported, clunky           | Fully integrated, fluent MIME support      |
| Authentication      | Basic, Bearer                 | Basic, Digest               | Basic, Bearer, Digest (incl. SHA-256)      |
| Build & Packaging   | Shared/static via CMake        | Static/shared library       | Header-only mode + Debian `.deb` packaging |
| Test Coverage       | Minimal                      | Minimal                    | Full coverage with Doctest + CI             |
| Thread Safety       | Not guaranteed                | Not guaranteed             | Not thread-safe (documented)                |
| Customization       | Limited                      | Moderate                   | Extensive fine-tuned libcurl control        |
| Docs & Examples     | Good docs, simple examples    | Sparse                     | Rich examples + auto-generated Doxygen docs|
| Community           | Active, popular              | Aging, low activity        | Growing with CI, automation, and tests      |


### Why Choose Curling Over CPR or curlpp?

More control, less complexity: Full access to advanced libcurl options via a modern interface.

RAII-safe: Automatic cleanup prevents memory and resource leaks.

Rich MIME and auth support: Complex uploads and multiple authentication schemes are built-in.

Flexible builds: Choose between header-only use or .deb packages for installation.

CI and tests: Every commit is tested, ensuring high reliability.

Safe defaults: Redirects and verbose logging are disabled by default, encouraging secure usage.



---

## 📚 Documentation

Generate API documentation with Doxygen:

```bash
make doc
```

HTML output will appear in the doc/ folder.

To clean:

```bash
make doc-clean
```

---

## 🧪 Testing

Tests use Doctest and cover:

✅ HTTP verbs: GET, POST, PUT, PATCH, DELETE, MIME

✅ Authentication: Basic, Bearer, Digest (MD5, SHA-256, auth-int)

✅ File download and upload

✅ Header manipulation

✅ JSON and form-data handling

✅ Redirect handling


Run locally:

```bash
make test
```

GitHub Actions ensures tests pass on every push and pull request.


---

## 🤝 Contributing

Contributions are welcome! Please:

Format code consistently

Run make test before pushing

Use atomic and focused commits



---

## 📄 License

This project is licensed under the MIT License.


---

## 👤 Maintainer

Paul Caron


---

## 📌 Notes

Curl global init/cleanup is handled automatically.

Not thread-safe — avoid sharing curling::Request across threads.

MIME is a distinct HTTP method type (not used with POST/PUT).

Cookie persistence defaults to cookies.txt, but is configurable.



---

