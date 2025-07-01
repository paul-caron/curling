#include <iostream>
#include <string>
#include <cstdlib>
#include "curling.hpp"

std::string get_api_key() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (!key) {
        std::cerr << "❌ OPENAI_API_KEY environment variable not set.\n";
        std::exit(1);
    }
    return key;
}

void call_chat(const std::string& api_key) {
    std::string prompt;
    std::cout << "Enter your message: ";
    std::getline(std::cin, prompt);

    std::string body = R"({
        "model": "gpt-4",
        "messages": [
            {"role": "user", "content": ")" + prompt + R"("}
        ]
    })";

    curling::Request req;
    req.setMethod(curling::Request::Method::POST)
       .setURL("https://api.openai.com/v1/chat/completions")
       .addHeader("Content-Type: application/json")
       .setAuthToken(api_key)
       .setBody(body);

    auto res = req.send();
    std::cout << "🧠 Response:\n" << res.body << "\n";
}

void call_completion(const std::string& api_key) {
    std::string prompt;
    std::cout << "Enter your prompt: ";
    std::getline(std::cin, prompt);

    std::string body = R"({
        "model": "text-davinci-003",
        "prompt": ")" + prompt + R"(",
        "max_tokens": 150
    })";

    curling::Request req;
    req.setMethod(curling::Request::Method::POST)
       .setURL("https://api.openai.com/v1/completions")
       .addHeader("Content-Type: application/json")
       .setAuthToken(api_key)
       .setBody(body);

    auto res = req.send();
    std::cout << "📝 Response:\n" << res.body << "\n";
}

void call_image_generation(const std::string& api_key) {
    std::string description;
    std::cout << "Describe the image to generate: ";
    std::getline(std::cin, description);

    std::string body = R"({
        "prompt": ")" + description + R"(",
        "n": 1,
        "size": "512x512"
    })";

    curling::Request req;
    req.setMethod(curling::Request::Method::POST)
       .setURL("https://api.openai.com/v1/images/generations")
       .addHeader("Content-Type: application/json")
       .setAuthToken(api_key)
       .setBody(body);

    auto res = req.send();
    std::cout << "🖼️ Response:\n" << res.body << "\n";
}

void list_models(const std::string& api_key) {
    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL("https://api.openai.com/v1/models")
       .setAuthToken(api_key);

    auto res = req.send();
    std::cout << "📦 Available Models:\n" << res.body << "\n";
}

int main() {
    std::string api_key = get_api_key();

    while (true) {
        std::cout << R"(
======== OpenAI CLI ========
1. Chat with GPT
2. Text Completion
3. Generate Image
4. List Models
0. Exit
> )";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") call_chat(api_key);
        else if (choice == "2") call_completion(api_key);
        else if (choice == "3") call_image_generation(api_key);
        else if (choice == "4") list_models(api_key);
        else if (choice == "0") break;
        else std::cout << "Invalid option.\n";
    }

    return 0;
}


