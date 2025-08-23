#include <iostream>
#include <cassert>
#include "../include/config.h"

// A simple test runner framework
void run_test(void (*test_function)(), const char* test_name) {
    std::cout << "Running test: " << test_name << "..." << std::endl;
    test_function();
    std::cout << test_name << ": PASSED" << std::endl;
}

// Test case: Config class should load default values when no file is present.
void test_load_defaults() {
    // Note: The Config class constructor tries to open a file.
    // We pass a non-existent file to ensure it loads defaults.
    Config config("non_existent_config.yaml");
    const AppConfig& appConfig = config.get();

    assert(appConfig.agent_path == "src/agent.py");
    assert(appConfig.log_file == "parent.log");
    assert(appConfig.log_level == INFO);
    assert(appConfig.post_action_wait_seconds == 15);
    assert(appConfig.schedule.size() == 1);
    assert(appConfig.schedule[0].start_hour == 0);
    assert(appConfig.schedule[0].end_hour == 23);
}

int main() {
    std::cout << "--- Starting C++ Unit Tests for Config ---" << std::endl;
    run_test(test_load_defaults, "test_load_defaults");
    std::cout << "--- All Config Tests Passed ---" << std::endl;
    return 0;
}
