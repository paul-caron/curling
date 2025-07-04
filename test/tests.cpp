#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../vendor/doctest/doctest.h"
#include "../include/curling.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <curl/curlver.h>
#include <curl/curl.h>
#include <cstdlib>
#include <sstream>
#include <regex>

int testN{1};

#define OYE std::cout << std::setw(2) << testN++ << " - " << doctest::detail::g_cs->currentTest->m_name << std::endl;


using namespace curling;

TEST_CASE("Library version check") {
    OYE
    std::string version = curling::version();
    CHECK(version == "1.2.0");
}

TEST_CASE("Request retries given number of attempts and fails on final one") {
    OYE
    Request req;
    req.setURL("http://nonexistent.localhost") // This domain won't resolve
       .setMethod(Request::Method::GET)
       .setTimeout(1)
       .setConnectTimeout(1);

    // Expect failure after 3 retries
    CHECK_THROWS_AS(req.send(3), RequestException);
}

TEST_CASE("Request honors port with non‑HTTP protocol (FTP)") {
    OYE
    using namespace curling;
    // Example using GNU's FTP server listening on default FTP port 21
    const std::string host = "ftp.gnu.org";
    const int port = 21;

    Request req;
    req.setURL("ftp://" + host + ":" + std::to_string(port) + "/")
       .setMethod(Request::Method::GET)  // GET works for FTP directory listing
       .setTimeout(10);

    try {
        Response res = req.send();
        CHECK(res.httpCode >= 100);  // FTP response is translated to numeric code
    } catch (const RequestException& ex) {
        FAIL("FTP request failed: " << ex.what());
    }
}


TEST_CASE("Simple Tor proxy test - GET through Tor SOCKS5 proxy") {
    OYE

    curling::Request req;
    req.setURL("https://check.torproject.org/api/ip")
       .setProxy("socks5h://127.0.0.1:9050")  // Tor SOCKS5 proxy (default Tor port)
       .enableVerbose(false);

    try {
        auto res = req.send();
        CHECK(res.httpCode == 200);
        // The response contains your Tor exit node IP if successful
        std::cout << "Tor IP response:\n" << res.body << std::endl;
        CHECK(res.body.find("IsTor") != std::string::npos);
    } catch (const curling::RequestException& ex) {
        FAIL("Request through Tor proxy failed: " << ex.what());
    }
}




TEST_CASE("Progress callback aborts download") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/stream-bytes/10000000") // large stream
       .setTimeout(10)
       .setProgressCallback([](curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
           return true; // abort immediately
       })
       .enableVerbose(false);

    CHECK_THROWS_AS(req.send(), curling::RequestException);
}







TEST_CASE("Reusing Request object with different URLs and methods") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/get")
       .setMethod(curling::Request::Method::GET)
       .enableVerbose(false);

    auto res1 = req.send();
    CHECK(res1.httpCode == 200);

    req.setURL("https://httpbin.org/put")
       .setMethod(curling::Request::Method::PUT)
       .setBody("Updated")
       .addHeader("Content-Type: text/plain");

    auto res2 = req.send();
    CHECK(res2.httpCode == 200);
    CHECK(res2.body.find("Updated") != std::string::npos);
}

TEST_SUITE("User Agent Tests"){
TEST_CASE("Custom User-Agent header test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/user-agent")
       .addHeader("User-Agent: CurlingTestClient/42.0")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("CurlingTestClient/42.0") != std::string::npos);
}

TEST_CASE("User-Agent set via setUserAgent method") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/user-agent")
       .setUserAgent("CurlingUserAgent/1.0")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("CurlingUserAgent/1.0") != std::string::npos);
}
}



TEST_CASE("Redirect not followed test") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/redirect-to?url=https://httpbin.org/get")
       .setFollowRedirects(false)
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 302);

    // Check that the "Location" header exists
    const auto& headers = res.headers;
    bool hasLocation = false;
    for (const auto& h : headers) {
        const std::string& key = h.first;
        if (key == "Location" || key == "location") {
            hasLocation = true;
            break;
        }
    }

    CHECK(hasLocation);
}



TEST_CASE("Invalid URL test") {
    OYE
    curling::Request req;
    req.setURL("http://nonexistent.curling.test.fake")
       .setTimeout(3)
       .enableVerbose(false);

    CHECK_THROWS_AS(req.send(), curling::RequestException);
}

TEST_CASE("Cookie persistence") {
    OYE
    const char * github = std::getenv("GITHUB");
    if(github){
        // test case doesnt work on github
        std::cout << "[doctest] Skipping cookie test inside CI environment\n";
        return;
    }

    const std::string cookieFile = "/tmp/cookies.txt";

    // Write cookie
    {
        curling::Request req;
        req.setURL("https://httpbin.org/cookies/set/mycookie/value")
           .setCookiePath(cookieFile)
           .setFollowRedirects(true)
           .enableVerbose(false);
        auto res = req.send();
        CHECK(res.httpCode == 200);
    }

    // Read cookie
    {
        curling::Request req;
        req.setURL("https://httpbin.org/cookies")
           .setCookiePath(cookieFile)
           .enableVerbose(false);

        auto res = req.send();
        CHECK(res.httpCode == 200);
        CHECK(res.body.find("mycookie") != std::string::npos);
        CHECK(res.body.find("value") != std::string::npos);
    }

    std::filesystem::remove(cookieFile);
}

TEST_CASE("Cookie persistence - Reusing the same object") {
    OYE
    const char * github = std::getenv("GITHUB");
    if(github){
        // test case doesnt work on github
        std::cout << "[doctest] Skipping cookie test inside CI environment\n";
        return;
    }

    const std::string cookieFile = "/tmp/cookies2.txt";

    // Write cookie

    curling::Request req;
    req.setURL("https://httpbin.org/cookies/set/mycookie/value")
       .setCookiePath(cookieFile)
       .setFollowRedirects(true)
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);

    // Read cookie

    req.setURL("https://httpbin.org/cookies")
       .setCookiePath(cookieFile)
       .enableVerbose(false);

    res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("mycookie") != std::string::npos);
    CHECK(res.body.find("value") != std::string::npos);

    std::filesystem::remove(cookieFile);
}

TEST_SUITE("Timeout tests"){
TEST_CASE("Connect timeout test") {
    OYE
    curling::Request req;
    req.setURL("https://10.255.255.1")  // unreachable IP
       .setConnectTimeout(2)
       .setTimeout(10)
       .enableVerbose(false);

    CHECK_THROWS_AS(req.send(), curling::RequestException);
}

TEST_CASE("Timeout test") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/delay/5") // waits 5 seconds
       .setTimeout(2)                         // timeout set to 2s
       .enableVerbose(false);

    CHECK_THROWS_AS(req.send(), curling::RequestException);
}
}



TEST_CASE("GET request to download image from httpbin") {
    OYE
    const std::string imageUrl = "https://httpbin.org/image/png";
    const std::string outputFile = "downloaded_image.png";

    curling::Request req;
    req.setURL(imageUrl)
       .enableVerbose(false)
       .downloadToFile(outputFile);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(std::filesystem::exists(outputFile));
    CHECK(std::filesystem::file_size(outputFile) > 0);

    std::filesystem::remove(outputFile);
}


TEST_SUITE("Athentication"){
TEST_CASE("GET request test with basic authentication") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/basic-auth/myusername/mypassword")
       .setHttpAuthMethod(curling::Request::AuthMethod::BASIC)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with bearer token auth") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/bearer")
       .setAuthToken("mytokenstring")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth/myusername/mypassword")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method and integrity protection") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth-int/myusername/mypassword")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method and MD5") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth/myusername/mypassword/md5")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method with integrity protection and MD5") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth-int/myusername/mypassword/md5")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method and SHA-256") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth/myusername/mypassword/SHA-256")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method with integrity protection and SHA-256") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth-int/myusername/mypassword/SHA-256")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .addHeader("User-Agent: CurlingClient/1.2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}
}


TEST_SUITE("testing HTTP methods"){

TEST_CASE("GET request test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/get")
       .addArg("key", "value")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"key\": \"value\"") != std::string::npos);
}

TEST_CASE("POST XML payload") {
    OYE
    curling::Request req;

    req.setMethod(curling::Request::Method::POST)
       .setURL("https://httpbin.org/post")
       .setBody(R"(
           <note>
               <to>User</to>
               <from>ChatGPT</from>
               <heading>Reminder</heading>
               <body>Don't forget to test your XML payload!</body>
           </note>
       )")
       .addHeader("Content-Type: application/xml");

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("<note>") != std::string::npos);
    CHECK(res.body.find("Don't forget to test your XML payload!") != std::string::npos);
}

TEST_CASE("POST request test with JSON body") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::POST)
       .setURL("https://httpbin.org/post")
       .addHeader("Content-Type: application/json")
       .setBody(R"({"name":"chatgpt","type":"AI"})")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find(R"("name": "chatgpt")") != std::string::npos);
    CHECK(res.body.find(R"("type": "AI")") != std::string::npos);
}

TEST_CASE("PUT request test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::PUT)
       .setURL("https://httpbin.org/put")
       .addHeader("Content-Type: text/plain")
       .setBody("Hello PUT")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("Hello PUT") != std::string::npos);
}

TEST_CASE("PATCH request test with JSON body") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::PATCH)
       .setURL("https://httpbin.org/patch")
       .addHeader("Content-Type: application/json")
       .setBody(R"({"name":"Lizardzilla","type":"Monster"})")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find(R"("name": "Lizardzilla")") != std::string::npos);
    CHECK(res.body.find(R"("type": "Monster")") != std::string::npos);
}

TEST_CASE("DELETE request test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::DEL)
       .setURL("https://httpbin.org/delete")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"url\": \"https://httpbin.org/delete\"") != std::string::npos);
}

TEST_CASE("HEAD request test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::HEAD)
       .setURL("https://httpbin.org/get")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.empty());
}
}

TEST_CASE("Redirect follow test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/redirect/1")
       .setFollowRedirects(true)
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"url\": \"https://httpbin.org/get\"") != std::string::npos);
}

TEST_CASE("Headers test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/headers")
       .addHeader("X-Test-Header: 123")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"X-Test-Header\": \"123\"") != std::string::npos);
}

TEST_CASE("Form-data (multipart) test") {
    OYE
    curling::Request req;
    req.setMethod(curling::Request::Method::MIME)
       .setURL("https://httpbin.org/post")
       .addFormField("field1", "value1")
       .addFormField("field2", "value2")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("value1") != std::string::npos);
    CHECK(res.body.find("value2") != std::string::npos);
}

TEST_SUITE("MIME Multipart tests"){
TEST_CASE("Multipart form with file upload") {
    OYE
    const char * github = std::getenv("GITHUB");
    if(github){
        // test case doesnt work on github
        std::cout << "[doctest] Skipping file upload test inside CI environment\n";
        return;
    }
    const std::string testFile = "/tmp/test_upload.txt";
    std::ofstream(testFile) << "This is test content";

    curling::Request req;
    req.setMethod(curling::Request::Method::MIME)
       .setURL("https://httpbin.org/post")
       .addFormField("field", "value")
       .addFormFile("file", testFile)
       .enableVerbose(false);

    auto res = req.send();
    CHECK(res.httpCode == 200);
    CHECK(res.body.find("value") != std::string::npos);
    CHECK(res.body.find("This is test content") != std::string::npos); // httpbin includes filename

    std::filesystem::remove(testFile);
}
}

TEST_CASE("Force HTTP/1.1 version") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/get")
       .setHttpVersion(curling::Request::HttpVersion::HTTP_1_1)
       .enableVerbose(false);

    auto res = req.send();
    CHECK(res.httpCode == 200);
}

TEST_CASE("Force HTTP/2 version") {
    OYE
    curling::Request req;
    req.setURL("https://httpbin.org/get")
       .setHttpVersion(curling::Request::HttpVersion::HTTP_2)
       .enableVerbose(false);

    auto res = req.send();
    CHECK(res.httpCode == 200);
}
