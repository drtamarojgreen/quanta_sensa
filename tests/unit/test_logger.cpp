#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "../include/logger.h"

// A simple test runner framework
void run_test(void (*test_function)(), const char* test_name) {
    std::cout << "Running test: " << test_name << "..." << std::endl;
    test_function();
    std::cout << test_name << ": PASSED" << std::endl;
}

// Helper to read a file's content
std::string read_all_from_file(const std::string& filename) {
    std::ifstream file(filename);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Test case: Logger should only log messages at or above its configured level.
void test_log_levels() {
    const std::string test_log_file = "test_level.log";
    // 1. Test with INFO level
    {
        Logger logger(test_log_file, INFO);
        logger.log(DEBUG, "This should not be logged.");
        logger.log(INFO, "This is an info message.");
        logger.log(ERROR, "This is an error message.");
    } // Logger goes out of scope, file is closed.

    std::string content = read_all_from_file(test_log_file);
    assert(content.find("[DEBUG]") == std::string::npos);
    assert(content.find("[INFO] This is an info message.") != std::string::npos);
    assert(content.find("[ERROR] This is an error message.") != std::string::npos);

    std::remove(test_log_file.c_str()); // Clean up
}

// Test case: Logger should format messages correctly.
void test_log_formatting() {
    const std::string test_log_file = "test_format.log";
    {
        Logger logger(test_log_file, DEBUG);
        logger.log(INFO, "Test message");
    }

    std::string content = read_all_from_file(test_log_file);
    // Example format: [2023-10-27 10:00:00] [INFO] Test message
    assert(content[0] == '['); // Starts with a bracket (for timestamp)
    assert(content.find("] [INFO] Test message") != std::string::npos);
    assert(content.back() == '\n');

    std::remove(test_log_file.c_str()); // Clean up
}


int main() {
    std::cout << "--- Starting C++ Unit Tests for Logger ---" << std::endl;
    run_test(test_log_levels, "test_log_levels");
    run_test(test_log_formatting, "test_log_formatting");
    std::cout << "--- All Logger Tests Passed ---" << std::endl;
    return 0;
}
