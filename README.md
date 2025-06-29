Sure! Here's your full README with the updated, elaborated Comparison section integrated exactly as you requested:

![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg) ![Tests](https://img.shields.io/badge/tests-passing-brightgreen) [![Build and Test](https://github.com/paul-caron/curling/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/paul-caron/curling/actions/workflows/build-and-test.yml) ![C++17](https://img.shields.io/badge/C%2B%2B-17-blue) ![GitHub release](https://img.shields.io/github/v/release/paul-caron/curling?include_prereleases&sort=semver)

# 🌀 Curling

**Curling** is a modern C++17 wrapper around **libcurl**, designed to simplify HTTP/HTTPS requests using a clean, fluent API.

It supports JSON payloads, file uploads, cookie management, authentication, proxy configuration, and more — all with a RAII-safe design using smart pointers.

---

## 📚 Table of Contents

- [✨ Features](#-features)
- [🛠 Installation](#-installation)
- [🚀 Basic Usage](#-basic-usage)
- [✅ Example Test Case](#-example-test-case)
- [🧠 Internals & Design](#-internals--design)
- [🤔 Why Curling?](#-why-curling)
- [📚 Documentation](#-documentation)
- [🧪 Testing](#-testing)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)
- [👤 Maintainer](#-maintainer)
- [📌 Notes](#-notes)

---

## ✨ Features

- 🔁 **Fluent API** — chainable and expressive
- 📤 **Multipart/MIME support** — for file uploads
- 🍪 **Cookie management** — with persistent storage
- 🛡 **Proxy and authentication support** — including basic, bearer, and digest
- 🌐 **Full HTTP verb support** — `GET`, `POST`, `PUT`, `DELETE`, `PATCH`, `MIME`
- 📦 **Linux `.deb` packaging**
- 🧪 **CI-tested** with Doctest framework and GitHub Actions

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

🪶 Header-only version

```bash
make header-only
```

This produces a header-only version in:
```
curling/header-only/
```

🚀 Basic Usage

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

🔨 Compile

With shared library:

```bash
g++ main.cpp -lcurling
```

With header-only:

```bash
g++ main.cpp -lcurl
```

✅ Example Test Case

Tests run automatically on every push via GitHub Actions. Example test case:

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

Run tests locally with:

```bash
make test
```

---

🧠 Internals & Design

Curling centers around the curling::Request class, which wraps libcurl functionality in a fluent, type-safe, and modern C++ API.

Key principles:

✅ RAII & smart pointers — automatic resource cleanup

✅ Fluent chaining — readable and efficient method calls

✅ No global state — avoids curl_global_* leaks

✅ Safe-by-default — redirects off by default, verbose off, etc.



---

🤔 Why Curling?

Libcurl is powerful, but its C API is verbose and error-prone.

Curling offers:

Feature	libcurl	Curling

Fluent C++ API	❌	✅
RAII memory management	❌	✅
Built-in test coverage	❌	✅
Easy file & MIME upload	Manual	✅
Modern build integration	❌	✅



---

⚖️ Comparisons with Other Popular C++ libcurl Wrappers

Curling is designed to strike a balance between modern C++ design, fine-grained control, and usability. Below is a detailed comparison with two well-known C++ libcurl wrappers: CPR and curlpp.

Aspect	CPR (GitHub)	curlpp (GitHub)	Curling (this project)

API Style	High-level, simple, intuitive	Classic, C++ wrapper but less modern	Modern C++17 fluent API with method chaining
Level of Abstraction	Heavy abstraction hides libcurl internals	Moderate abstraction but somewhat outdated	Balanced abstraction exposing fine libcurl control
Memory Management	Manual (raw pointers internally)	Manual	RAII-based, smart pointers for safety
Feature Completeness	Covers common HTTP needs: GET, POST, JSON, basic auth, etc.	Covers many libcurl features but API is less ergonomic	Full HTTP verbs, MIME uploads, advanced auth
File & MIME Uploads	Supports file uploads, but API is limited and sometimes verbose	Supports MIME but API is clunky	Fully integrated MIME support with fluent syntax
Authentication Support	Basic and Bearer token auth	Basic, Digest	Basic, Bearer, Digest (MD5, SHA-256, auth-int)
Build & Packaging	Single static/shared library, typical CMake build	Single static/shared library	Flexible: header-only mode + Debian .deb packaging
Test Coverage	Limited or external	Minimal	Built-in test coverage using Doctest + CI
Thread Safety	Not guaranteed	Not guaranteed	Not thread-safe; explicitly documented
Customization & Control	Limited access to low-level libcurl options	Moderate	Extensive control, no global state leaks, safe defaults
Documentation & Examples	Good user guide and examples	Sparse	Detailed examples, generated docs via Doxygen
Community & Activity	Active, popular	Less active, older	Growing project with CI and regular updates



---

Why Choose Curling Over CPR or curlpp?

More control without complexity: Curling exposes advanced libcurl options in a modern C++ style, letting you customize requests fully without sacrificing ease of use.

RAII safety: Automatic resource management prevents common memory and handle leaks present in manual wrappers.

Rich MIME and auth support: Supports complex file uploads and multiple auth schemes (including modern digest variants) out of the box.

Modern build flexibility: Choose between a lightweight header-only mode or a packaged Debian .deb, fitting your project needs.

Built-in testing and CI: Continuous integration with test coverage ensures reliability and ease of maintenance.

Safe defaults: Curling disables potentially unsafe defaults like automatic redirects, encouraging explicit, secure code.



---

Summary

Feature	CPR	curlpp	Curling

Fluent API	✅	❌	✅
RAII memory management	❌	❌	✅
Advanced MIME support	Partial	Partial	Full
Authentication schemes	Basic, Bearer	Basic, Digest	Basic, Bearer, Digest
Build options	Standard library build	Standard library build	Header-only + Debian .deb
Test coverage	Minimal	Minimal	Full, CI tested
Fine-grained control	Limited	Moderate	Extensive
Modern C++17 idioms	Partial	Older API	Full C++17



---

If your project demands modern C++ safety, fine control of HTTP requests, and robust MIME/file upload support, Curling provides a compelling alternative to CPR and curlpp, without sacrificing performance or flexibility.


---

📚 Documentation

Generate docs using Doxygen:

make doc

HTML output will appear in the doc/ folder.

To clean:

make doc-clean

🧪 Testing

Tests use [Doctest](https://github.com/doctest/doctest) and cover:

- ✅ HTTP verbs: GET, POST, PUT, PATCH, DELETE, MIME
- ✅ Authentication: Basic, Bearer, Digest (including MD5, SHA-256, and `auth-int`)
- ✅ File download and upload
- ✅ Header manipulation
- ✅ JSON and form-data handling
- ✅ Redirect handling

To run tests locally:

```bash
make test
```

GitHub Actions ensures all tests pass on every push and pull request, providing continuous integration and maintaining project stability.


---

🤝 Contributing

Contributions are welcome! Feel free to open issues, propose features, or submit pull requests.

Before submitting a PR:

Format code consistently using the project's style guidelines

Ensure all tests pass by running make test

Prefer atomic and focused commits that are easier to review and maintain



---

📄 License

This project is licensed under the MIT License.


---

👤 Maintainer

Paul Caron
GitHub Profile


---

📌 Notes

Curl global initialization and cleanup are handled automatically.

This library is not thread-safe; avoid sharing curling::Request instances across threads.

MIME is treated as a distinct HTTP method type and should not be mixed with POST or PUT.

Cookie persistence defaults to the file cookies.txt, but this is configurable.



---
