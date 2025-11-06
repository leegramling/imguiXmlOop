#pragma once

#include <string>
#include <vector>

/**
 * @brief Represents a single city's data for the UI
 */
struct CityData {
    std::string name;
    float latitude = 0.0f;
    float longitude = 0.0f;
    int elevation = 0;      // meters above sea level
    float avg_temp = 0.0f;  // Celsius
    int population = 0;
    int climate_zone = 0;   // 0=Temperate, 1=Tropical, 2=Arid, 3=Continental
};

/**
 * @brief Application data structure shared between XML and builder flows
 */
struct AppData {
    std::string name;
    std::string email;
    bool python_selected = false;
    bool go_selected = false;
    bool swift_selected = false;
    bool rust_selected = false;
    bool cpp_selected = false;

    // City data for grid
    std::vector<CityData> cities;
};
