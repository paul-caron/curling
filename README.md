![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)
![Tests](https://img.shields.io/badge/tests-passing-brightgreen)
[![Build and Test](https://github.com/paul-caron/curling/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/paul-caron/curling/actions/workflows/build-and-test.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![GitHub release](https://img.shields.io/github/v/release/paulcaron/curling?include_prereleases&sort=semver)

# Curling

**Curling** is a modern C++17 wrapper around **libcurl**, designed to simplify HTTP/HTTPS requests using a clean and fluent API.

It supports common web request features including JSON payloads, file uploads, cookies, proxy configuration, and more — all while safely managing libcurl resources using smart pointers and RAII.

---

## ✨ Features

- 🔁 **Fluent API** — chainable and expressive
- 📤 **Multipart/MIME support** — for file uploads
- 🍪 **Cookie management** — with persistent storage
- 🛡 **Proxy and authentication support**
- 🌐 **Full HTTP verb support** — `GET`, `POST`, `PUT`, `DELETE`, `PATCH`, `MIME`
- 📦 **Linux `.deb` package support**
- 🧪 CI-tested

---

## 🛠 Installation

### 🔧 Build with `make`

```bash
make
```

To install to a staged Debian package structure:

```bash
make install
make deb
```
This will produce a .deb package named:

```bash
curling_1.0_amd64.deb
```

Use apt to install and update ld cache:
```
sudo apt install ./curling_1.0_amd64.dev
sudo ldconfig
```

---

## 🚀 Basic Usage

```cpp
//main.cpp
#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;

    auto res = req.setMethod(curling::Request::Method::GET)
                  .setURL("https://example.com")
                  .setUserAgent("Lizardzilla/6.9 (Reptillian Humanoid)")
                  .send();

    std::cout << res.toString();
    return 0;
}
```
### compile
```bash
g++ main.cpp -lcurling
```

---

## 📄 License

MIT License




---

## 👤 Maintainer

Paul Caron


---

## 📌 Notes

This library handles curl_global_init / curl_global_cleanup internally.

Not thread-safe — do not share curling::Request instances across threads.

MIME requests (for file uploads) are mutually exclusive with other HTTP methods.

Default cookie file is cookies.txt, configurable with .setCookiePath().



---

## 📚 Documentation

Generated with Doxygen:
```bash
make doc
```
Output will be in the doc/ folder. Clean with:
```bash
make doc-clean
```

---

## ✅ Testing

This project uses GitHub Actions workflows to automate building and testing on every push and pull request. The workflow is defined in .github/workflows/build-and-test.yml and ensures that all tests pass before changes are merged.

### Running Tests Locally

To run tests locally, use make test:
```
make test
```

---
