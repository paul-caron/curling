#include <curling.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp> //json response parsing

void print_usage() {
    std::cout << "Usage: weather_tool --lat <latitude> --lon <longitude>\n";
}

int main(int argc, char* argv[]) {
    std::string lat, lon;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--lat" && i + 1 < argc) lat = argv[++i];
        else if (arg == "--lon" && i + 1 < argc) lon = argv[++i];
    }

    if (lat.empty() || lon.empty()) {
        print_usage();
        return 1;
    }

    try {
        curling::Request req;
        req.setMethod(curling::Request::Method::GET)
           .setURL("https://api.open-meteo.com/v1/forecast")
           .addArg("latitude", lat)
           .addArg("longitude", lon)
           .addArg("current_weather", "true");

        curling::Response res = req.send();

        std::cout << "HTTP Status: " << res.httpCode << "\n";

        // Parse the JSON response
        try {
            auto json = nlohmann::json::parse(res.body);
            const auto& weather = json["current_weather"];
            std::cout << "Temperature: " << weather["temperature"] << " °C\n";
            std::cout << "Windspeed: " << weather["windspeed"] << " km/h\n";
            std::cout << "Time: " << weather["time"] << "\n";
        } catch (...) {
            std::cout << "Raw response:\n" << res.body << "\n";
        }

    } catch (const curling::CurlingException& ex) {
        std::cerr << "Curling error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
