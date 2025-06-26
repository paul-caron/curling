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

curling_1.0_amd64.deb

🔮 Future Install Options (planned)

CMake

vcpkg

Conan



---

🚀 Basic Usage

```cpp
#include "curling.hpp"
#include <iostream>

int main() {
    curling::Request req;

    auto res = req.setMethod(curling::Request::Method::GET)
                  .setURL("https://example.com")
                  .send();

    std::cout << res.toString();
    return 0;
}
```

---

📄 License

MIT License


---

👤 Maintainer

Paul Caron


---

📌 Notes

This library handles curl_global_init / curl_global_cleanup internally.

Not thread-safe — do not share curling::Request instances across threads.

MIME requests (for file uploads) are mutually exclusive with other HTTP methods.

Default cookie file is cookies.txt, configurable with .setCookiePath().



---

📚 Documentation

Generated with Doxygen:

make doc

Output will be in the doc/ folder. Clean with:

make doc-clean


---


