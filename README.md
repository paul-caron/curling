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

- [✨ Features](#-features)
- [🛠 Installation](#-installation)
- [🚀 Basic Usage](#-basic-usage)
- [✅ Example Test Case](#-example-test-case)
- [🧠 Internals & Design](#-internals--design)
- [🤔 Why Curling?](#-why-curling)
- [⚖️ Comparisons](#️-comparisons-with-other-popular-c-libcurl-wrappers)
- [📚 Documentation](#-documentation)
- [🧪 Testing](#-testing)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)
- [👤 Maintainer](#-maintainer)
- [📌 Notes](#-notes)


---

## ✨ Features

- 🔁 **Fluent API** — chainable and expressive request building  
- 📤 **Multipart and MIME support** — for multipart forms and file uploads  
- 🍪 **Cookie management** — with optional persistent storage  
- 🛡 **Proxy and authentication support** — including Basic, Bearer, and Digest  
- 🌐 **Full HTTP verb support** — GET, POST, PUT, DELETE, PATCH, HEAD
- 🚀 **HTTP/2 and HTTP/3 support** — via libcurl
- ⏳ **Progress callback support** — for monitoring request progress
- 🧩 **Header-only library** — just include and go
- 📦 **.deb packaging** — for easy installation on Debian-based systems  
- 🧪 **CI-tested** — with [Doctest](https://github.com/doctest/doctest) and GitHub Actions



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
g++ main.cpp -lcurling -std=c++17
```
With header-only:
```bash
g++ main.cpp -lcurl -std=c++17
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

## 🧠 Design Philosophy

Curling centers around the curling::Request class, which wraps libcurl functionality in a fluent, type-safe, modern C++ API.

Key principles:

- ✅ RAII & smart pointers — automatic resource cleanup

- ✅ Fluent chaining — readable and efficient method calls

- ✅ No global state — avoids curl_global_* leaks

- ✅ Safe-by-default — redirects off by default, verbose off, etc.



---

## 🤔 Why Curling?

Libcurl is powerful, but its C API is verbose and error-prone.

Curling offers:

|Feature                 |libcurl    |Curling|
|------------------------|-----------|-------|
|Fluent C++ API          |❌         |✅     |
|RAII memory management	 |❌         |✅     |
|Built-in test coverage	 |❌	     |✅     |
|Easy file & MIME upload |Manual     |✅     |
|Modern build integration|❌         |✅     |


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

- ✅ HTTP verbs: GET, POST, PUT, PATCH, DELETE, MIME

- ✅ Authentication: Basic, Bearer, Digest (MD5, SHA-256, auth-int)

- ✅ File download and upload

- ✅ Header manipulation

- ✅ JSON and form-data handling

- ✅ Redirect handling


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

---

## 🫶 Please Look The Examples

Such as ollama.hpp :
```cpp
#pragma once
#include "curling.hpp"
#include "json.hpp"

#include <string>
#include <stdexcept>
#include <optional>
#include <vector>

class OllamaClient {
public:
    explicit OllamaClient(const std::string& baseUrl = "http://localhost:11434")
        : baseUrl_(baseUrl) {}

    struct ChatMessage {
        std::string role;
        std::string content;
    };

    struct ChatResponse {
        std::string model;
        std::string message;
    };

    struct GenerateResponse {
        std::string response;
    };

    // Send chat messages to the /api/chat endpoint
ChatResponse chat(const std::string& model,
                  const std::vector<ChatMessage>& messages,
                  const std::optional<std::string>& systemPrompt = std::nullopt,
                  const std::optional<nlohmann::json>& options = std::nullopt) {
        nlohmann::json payload;
        payload["model"] = model;
        payload["messages"] = nlohmann::json::array();
        payload["stream"] = false; // Disable streaming

        for (const auto& msg : messages) {
            payload["messages"].push_back({{"role", msg.role}, {"content", msg.content}});
        }

        if (systemPrompt.has_value()) {
            payload["system"] = systemPrompt.value();
        }

        if (options.has_value()) {
            payload["options"] = options.value();
        }

        curling::Request req;
        req.setMethod(curling::Request::Method::POST)
           .setURL(baseUrl_ + "/api/chat")
           .addHeader("Content-Type: application/json")
           .setBody(payload.dump());

        curling::Response res = req.send();

        if (res.httpCode != 200) {
            throw std::runtime_error("Chat request failed: " + std::to_string(res.httpCode) +
                                     " Body: " + res.body);
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(res.body);
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("Failed to parse chat response JSON: " + std::string(e.what()));
        }

        std::string message;
        if (json.contains("message") && json["message"].is_object()) {
            message = json["message"].value("content", "");
        }

        return ChatResponse{
            .model = json.value("model", ""),
            .message = message
        };
    }

    // Send a single prompt to the /api/generate endpoint
    GenerateResponse generate(const std::string& model,
                              const std::string& prompt,
                              const std::optional<nlohmann::json>& options = std::nullopt) {
        nlohmann::json payload;
        payload["model"] = model;
        payload["prompt"] = prompt;
        payload["stream"] = false; // Disable streaming


        if (options.has_value()) {
            payload["options"] = options.value();
        }

        curling::Request req;
        req.setMethod(curling::Request::Method::POST)
           .setURL(baseUrl_ + "/api/generate")
           .addHeader("Content-Type: application/json")
           .setBody(payload.dump());

        curling::Response res = req.send();

        if (res.httpCode != 200) {
            throw std::runtime_error("Generate request failed: " + std::to_string(res.httpCode) +
                                     " Body: " + res.body);
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(res.body);
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("Failed to parse generate response JSON: " + std::string(e.what()));
        }

        return GenerateResponse{
            .response = json.value("response", "")
        };
    }

private:
    std::string baseUrl_;
};
```
