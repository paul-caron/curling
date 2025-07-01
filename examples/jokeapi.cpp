#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "curling.hpp"

using json = nlohmann::json;

void fetchJoke(const std::string& category) {
    try {
        curling::Request req;
        req.setMethod(curling::Request::Method::GET)
           .setURL("https://v2.jokeapi.dev/joke/" + category)
           .addArg("format", "json")
           .addArg("safe-mode", ""); // Optional to avoid offensive jokes

        curling::Response res = req.send();

        if (res.httpCode == 200) {
            auto parsed = json::parse(res.body);
            std::cout << "\n--- Joke ---\n";
            if (parsed["type"] == "single") {
                std::cout << parsed["joke"] << std::endl;
            } else {
                std::cout << parsed["setup"] << "\n" << parsed["delivery"] << std::endl;
            }
        } else {
            std::cerr << "Failed to fetch joke. HTTP Code: " << res.httpCode << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void showMenu() {
    std::cout << "\n=== JokeAPI CLI ===\n"
              << "1. Programming\n"
              << "2. Misc\n"
              << "3. Dark\n"
              << "4. Pun\n"
              << "5. Spooky\n"
              << "6. Christmas\n"
              << "0. Exit\n"
              << "Choose a category: ";
}

int main() {
    int choice;
    while (true) {
        showMenu();
        std::cin >> choice;

        if (std::cin.fail() || choice < 0 || choice > 6) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid choice.\n";
            continue;
        }

        if (choice == 0) break;

        std::string category;
        switch (choice) {
            case 1: category = "Programming"; break;
            case 2: category = "Misc"; break;
            case 3: category = "Dark"; break;
            case 4: category = "Pun"; break;
            case 5: category = "Spooky"; break;
            case 6: category = "Christmas"; break;
        }

        fetchJoke(category);
    }

    std::cout << "Goodbye!\n";
    return 0;
}



