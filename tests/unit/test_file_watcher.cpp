#include <iostream>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <sys/stat.h>
#include "../../include/file_watcher.h"
#include "../../include/logger.h"

// Mock the global logger needed by file_watcher.cpp
Logger* logger = nullptr;

// A simple test runner framework (copied from other tests)
void run_test(void (*test_function)(), const char* test_name) {
    std::cout << "Running test: " << test_name << "..." << std::endl;
    test_function();
    std::cout << test_name << ": PASSED" << std::endl;
}

// Test case: FileWatcher should detect a new file creation.
void test_file_creation_event() {
    const std::string TEST_DIR = "temp_test_dir";
    const std::string TEST_FILE = TEST_DIR + "/test_file.txt";

    // 1. Setup
    mkdir(TEST_DIR.c_str(), 0777);
    ThreadSafeQueue<FileEvent> event_queue;
    std::vector<std::string> dirs_to_watch = {TEST_DIR};
    FileWatcher watcher(dirs_to_watch, event_queue);
    watcher.start();

    // Give the watcher a moment to start up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 2. Action: Create a file in the watched directory
    std::ofstream(TEST_FILE).close();

    // 3. Verification
    // Give the watcher a moment to detect the change
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    FileEvent event;
    bool event_found = false;
    // Poll the queue for a short time
    for (int i = 0; i < 10; ++i) {
        if (event_queue.try_pop(event)) {
            event_found = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    assert(event_found);
    assert(event.type == FileEventType::CREATE);
    assert(event.path == TEST_FILE);

    // 4. Teardown
    watcher.stop();
    remove(TEST_FILE.c_str());
    rmdir(TEST_DIR.c_str());
}

int main() {
    // Suppress console output from logger for clean test results
    logger = new Logger("/dev/null", ERROR);
    std::cout << "--- Starting C++ Unit Tests for FileWatcher ---" << std::endl;
    run_test(test_file_creation_event, "test_file_creation_event");
    std::cout << "--- All FileWatcher Tests Passed ---" << std::endl;
    delete logger;
    return 0;
}
