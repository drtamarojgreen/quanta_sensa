#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include "logger.h" // Include logger.h for the LogLevel enum

// Defines the structure for a single time window for the agent to run.
struct TimeWindow {
    int start_hour;
    int end_hour;
};

// Main configuration structure for the application.
struct AppConfig {
    std::string agent_path;
    std::string log_file;
    LogLevel log_level;
    std::vector<TimeWindow> schedule;
    int post_action_wait_seconds;
};

/**
 * @class Config
 * @brief Handles loading configuration from a YAML file.
 *
 * This class is responsible for parsing a configuration file (e.g., config.yaml)
 * and providing access to the application settings.
 *
 * @note This implementation assumes the presence of a YAML parsing library
 * like yaml-cpp. The actual parsing logic is a placeholder.
 */
class Config {
public:
    /**
     * @brief Constructs a Config object and loads the configuration from the given file.
     * @param filename The path to the configuration file.
     */
    Config(const std::string& filename);

    /**
     * @brief Returns the loaded application configuration.
     * @return A constant reference to the AppConfig structure.
     */
    const AppConfig& get() const;

private:
    AppConfig appConfig;

    /**
     * @brief Parses the YAML file and populates the AppConfig structure.
     * @param filename The path to the configuration file.
     */
    void loadFromFile(const std::string& filename);
};

#endif // CONFIG_H
