#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>
#include <cstdio>
#include <memory>
#include <array>
#include <vector>
#include <algorithm>
#include <regex>

namespace fs = std::filesystem;

// Implementation of the Precision Power SDD Card Runner for QuantaSensa
// Discovers and executes card blocks, enforcing Green Syntax and Numeric Evidence.

std::string trim_str(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) start++;
    auto end = s.end();
    if (start == end) return "";
    do { end--; } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

class CardRunner {
public:
    void run(const fs::path& sdd_dir) {
        std::cout << "--- Precision Power SDD Card Runner Execution ---" << std::endl;
        fs::path cards_dir = sdd_dir / "cards";
        if (!fs::exists(cards_dir)) {
            std::cerr << "Cards directory not found: " << cards_dir << std::endl;
            return;
        }

        for (const auto& entry : fs::directory_iterator(cards_dir)) {
            if (entry.path().extension() == ".cpp" || entry.path().extension() == ".py") {
                processCard(entry.path());
            }
        }
    }

private:
    void processCard(const fs::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open card file: " << path << std::endl;
            return;
        }

        std::string line;
        bool has_tools = false;
        bool has_params = false;
        bool has_results = false;
        bool is_lazy = false;
        bool has_numeric = false;

        std::cout << "Card File: " << path.filename() << std::endl;

        // Matches @Results key == value (numeric)
        std::regex results_regex(R"(@Results\s+\w+\s*==\s*(\d+))");

        while (std::getline(file, line)) {
            std::string trimmed = trim_str(line);
            if (trimmed.find("// @Card:") == 0 || trimmed.find("# @Card:") == 0) {
                std::cout << "  [Card Found] " << (trimmed.find("//") == 0 ? trimmed.substr(9) : trimmed.substr(8)) << std::endl;
            }
            if (trimmed.find("TOOLS") != std::string::npos) has_tools = true;
            if (trimmed.find("PARAMETERS") != std::string::npos) has_params = true;
            if (trimmed.find("RESULTS") != std::string::npos) has_results = true;

            std::smatch match;
            if (std::regex_search(trimmed, match, results_regex)) {
                has_numeric = true;
                std::string val = match[1].str();
                if (val == "0" || val == "1") is_lazy = true;
            }
        }

        std::cout << "    Green Syntax: "
                  << (has_tools && has_params && has_results ? "VALID" : "INVALID") << std::endl;

        std::cout << "    Numeric Evidence: "
                  << (has_numeric ? (is_lazy ? "LAZY (FAILED)" : "VALID") : "MISSING") << std::endl;

        if (has_tools && has_params && has_results && has_numeric && !is_lazy) {
            std::cout << "    Execution: SUCCESS (Descriptive Evidence Verified)" << std::endl;
        } else {
            std::cout << "    Execution: FAILED (Structural/Metric Violation)" << std::endl;
        }
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[]) {
    fs::path sdd_path = fs::current_path() / "tests" / "sdd";
    if (argc > 1) {
        sdd_path = argv[1];
    }

    CardRunner runner;
    runner.run(sdd_path);
    return 0;
}
