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
