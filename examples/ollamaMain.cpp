#include "ollama.hpp"
#include <iostream>

int main() {
    try {
        OllamaClient client;

        // Chat endpoint usage
        std::vector<OllamaClient::ChatMessage> messages = {
            {"user", "Hello! What's the weather like today?"}
        };

        auto chatRes = client.chat("llama3", messages);
        std::cout << "Chat response: " << chatRes.message << "\n";

        // Generate endpoint usage
        auto genRes = client.generate("llama3", "List 3 interesting facts about space.");
        std::cout << "Generate response: " << genRes.response << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
