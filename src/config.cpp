#include "../include/config.h"
#include "../include/logger.h" // For LogLevel enum
#include <iostream>
#include <fstream>

// --- MOCK YAML PARSING ---
// The following is a placeholder for a real YAML parsing library like yaml-cpp.
// In a real implementation, you would include the library's header and use its API.
// For example: #include <yaml-cpp/yaml.h>

/**
 * @brief Mock function to simulate parsing a YAML file.
 *
 * This function provides default values and simulates reading from a file.
 * In a real scenario, this would use a library to parse the file contents.
 */
static void parse_yaml_mock(const std::string& filename, AppConfig& config) {
    // Set default values first
    config.agent_path = "src/agent.py";
    config.log_file = "parent.log";
    config.log_level = INFO;
    config.post_action_wait_seconds = 15;
    config.schedule.push_back({0, 23}); // Default: run all day

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file '" << filename << "'. Using default values." << std::endl;
        return;
    }

    // In a real implementation, you would parse the YAML nodes here.
    // For example:
    // YAML::Node yaml = YAML::LoadFile(filename);
    // config.agent_path = yaml["agent_path"].as<std::string>();
    // ... and so on for all other settings.
    // Since we can't do that, we'll just log that we are using defaults
    // after "successfully" opening the file.
    std::cout << "Info: Config file found. (Parsing is mocked, using defaults)." << std::endl;
}


// --- Config Class Implementation ---

Config::Config(const std::string& filename) {
    loadFromFile(filename);
}

const AppConfig& Config::get() const {
    return appConfig;
}

void Config::loadFromFile(const std::string& filename) {
    // This is where the actual parsing happens.
    // We are using our mock function here.
    parse_yaml_mock(filename, this->appConfig);
}
