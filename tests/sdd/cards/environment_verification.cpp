#include <iostream>
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

// @Card: environment_verification
// TOOLS: g++, uname, python3
// PARAMETERS: None
// RESULTS: os_verified == 2, tool_count == 3
// @Results os_verified == 2

int main() {
    int os_verified = 0;
    int tool_count = 0;

    // Check OS
    #ifdef __linux__
        os_verified = 2;
    #endif

    // Verify tools existence (simulated for card)
    if (system("which g++ > /dev/null 2>&1") == 0) tool_count++;
    if (system("which uname > /dev/null 2>&1") == 0) tool_count++;
    if (system("which python3 > /dev/null 2>&1") == 0) tool_count++;

    std::cout << "os_verified == " << os_verified << std::endl;
    std::cout << "tool_count == " << tool_count << std::endl;

    return 0;
}
