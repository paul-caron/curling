#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../include/curling.hpp"
#include <iostream>

TEST_CASE("GET request test") {
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://httpbin.org/get")
       .addArg("key", "value")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"key\": \"value\"") != std::string::npos);
}

TEST_CASE("GET request test with basic authentication") {
    curling::Request req;
    req.setURL("https://httpbin.org/basic-auth/myusername/mypassword")
       .setHttpAuthMethod(curling::Request::AuthMethod::BASIC)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with bearer token auth") {
    curling::Request req;
    req.setURL("https://httpbin.org/bearer")
       .setAuthToken("mytokenstring")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method") {
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth/myusername/mypassword")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method and integrity protection") {
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth-int/myusername/mypassword")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method and MD5") {
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth/myusername/mypassword/md5")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method with integrity protection and MD5") {
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth-int/myusername/mypassword/md5")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method and SHA-256") {
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth/myusername/mypassword/SHA-256")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("GET request test with Digest authorization method with integrity protection and SHA-256") {
    curling::Request req;
    req.setURL("https://httpbin.org/digest-auth/auth-int/myusername/mypassword/SHA-256")
       .setHttpAuthMethod(curling::Request::AuthMethod::DIGEST)
       .setHttpAuth("myusername","mypassword")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"authenticated\": true") != std::string::npos);
}

TEST_CASE("POST request test with JSON body") {
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
    curling::Request req;
    req.setMethod(curling::Request::Method::DEL)
       .setURL("https://httpbin.org/delete")
       .enableVerbose(false);

    auto res = req.send();

    CHECK(res.httpCode == 200);
    CHECK(res.body.find("\"url\": \"https://httpbin.org/delete\"") != std::string::npos);
}

TEST_CASE("Redirect follow test") {
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

