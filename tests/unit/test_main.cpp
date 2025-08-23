#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>

// --- Mock Logger ---
// Create a mock of the Logger class to avoid file I/O during tests.
// This also allows us to inspect logged messages.
enum LogLevel { DEBUG, INFO, ERROR };
struct MockLogger {
    std::vector<std::string> messages;
    void log(LogLevel level, const std::string& msg) {
        messages.push_back(msg);
    }
};
// Global mock logger instance, similar to the real one in main.cpp
MockLogger* logger = nullptr;


// --- Function to be tested ---
// This is a copy of the function from main.cpp for isolated testing.
std::string read_file_content(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        if (logger) {
            logger->log(ERROR, "Failed to open file: " + path);
        }
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}


// --- Test Framework ---
void run_test(void (*test_function)(), const char* test_name) {
    // Setup: initialize the mock logger for each test
    logger = new MockLogger();
    std::cout << "Running test: " << test_name << "..." << std::endl;
    test_function();
    std::cout << test_name << ": PASSED" << std::endl;
    // Teardown
    delete logger;
    logger = nullptr;
}

// --- Test Cases ---

// Test case: read_file_content should read a file's content correctly.
void test_read_valid_file() {
    const std::string test_filename = "test_read.txt";
    const std::string expected_content = "Hello, this is a test.";
    std::ofstream(test_filename) << expected_content;

    std::string actual_content = read_file_content(test_filename);

    assert(actual_content == expected_content);
    assert(logger->messages.empty()); // No error should be logged

    std::remove(test_filename.c_str());
}

// Test case: read_file_content should return an empty string for a non-existent file.
void test_read_invalid_file() {
    const std::string test_filename = "non_existent_file.txt";

    std::string actual_content = read_file_content(test_filename);

    assert(actual_content.empty());
    assert(logger->messages.size() == 1); // An error should be logged
    assert(logger->messages[0].find("Failed to open file") != std::string::npos);
}

int main() {
    std::cout << "--- Starting C++ Unit Tests for Main ---" << std::endl;
    run_test(test_read_valid_file, "test_read_valid_file");
    run_test(test_read_invalid_file, "test_read_invalid_file");
    std::cout << "--- All Main Tests Passed ---" << std::endl;
    return 0;
}
