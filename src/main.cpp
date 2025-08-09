#include "../include/logger.h"
#include "../include/config.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <sys/ioctl.h>
#include <ctime>
#include <fcntl.h>      // For fcntl
#include <dirent.h>     // For directory scanning
#include <fstream>      // For file reading
#include <atomic>       // For the pause signal

// --- Globals ---
Logger* logger = nullptr;
const Config* config = nullptr;
std::atomic<bool> exit_flag(false); // Global flag to signal exit

#include <cstdio>       // For popen

// --- Constants ---
const std::string ACTIONS_PENDING_DIR = "actions/pending/";
const std::string ACTIONS_IN_PROGRESS_DIR = "actions/in_progress/";
const std::string ACTIONS_FAILED_DIR = "actions/failed/";
const std::string QUEUE_COMPLETED_DIR = "queue/completed/";
const std::string QUEUE_FAILED_DIR = "queue/failed/";
const std::string ETHOS_VALIDATOR_PATH = "./quanta-ethos";

// --- Data Structures ---
struct ScriptResult {
    int exit_code;
    std::string stdout_output;
    std::string stderr_output;
};

// --- Function Prototypes ---
void set_terminal_raw(bool raw);
int kbhit();
std::string read_from_pipe(int fd);
std::string find_script_in_pending();
std::string read_file_content(const std::string& path);
void process_script(const std::string& script_filename);
void listen_for_exit();
bool validate_with_ethos(const std::string& script_content, std::string& reason);
void write_to_file(const std::string& path, const std::string& content);
ScriptResult execute_script(const std::string& script_path);


/**
 * @brief Main entry point for the parent controller.
 */
int main(int argc, char* argv[]) {
    config = new Config("config.yaml");
    const AppConfig& appConfig = config->get();
    logger = new Logger(appConfig.log_file, appConfig.log_level);

    logger->log(INFO, "Parent controller started. Polling for scripts... Press ESC to exit.");

    // Start a separate thread to listen for the ESC key
    std::thread exit_listener(listen_for_exit);

    while (!exit_flag) {
        std::string script_filename = find_script_in_pending();
        if (!script_filename.empty()) {
            logger->log(INFO, "Found script: " + script_filename);
            process_script(script_filename);
        }

        // Wait for a short interval before polling again
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    logger->log(INFO, "Exit flag set. Shutting down.");
    exit_listener.join(); // Wait for the listener thread to finish

    delete logger;
    delete config;
    return 0;
}

/**
 * @brief Scans the actions/pending directory for the first script file.
 * @return The filename of the script, or an empty string if none found.
 */
std::string find_script_in_pending() {
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(ACTIONS_PENDING_DIR.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string filename = ent->d_name;
            // Ignore '.' and '..' directories and hidden files
            if (filename != "." && filename != ".." && filename[0] != '.') {
                closedir(dir);
                return filename;
            }
        }
        closedir(dir);
    } else {
        // This can happen if the directory doesn't exist. Log it once.
        static bool error_logged = false;
        if (!error_logged) {
            logger->log(ERROR, "Could not open directory: " + ACTIONS_PENDING_DIR);
            error_logged = true;
        }
    }
    return "";
}

/**
 * @brief Processes a single script found in the pending queue.
 * @param script_filename The name of the script file in the actions/pending/ dir.
 */
void process_script(const std::string& script_filename) {
    std::string pending_path = ACTIONS_PENDING_DIR + script_filename;
    std::string in_progress_path = ACTIONS_IN_PROGRESS_DIR + script_filename;

    // Move script to in_progress
    if (std::rename(pending_path.c_str(), in_progress_path.c_str()) != 0) {
        logger->log(ERROR, "Failed to move script to in_progress: " + script_filename);
        return;
    }
    logger->log(INFO, "Moved script to " + in_progress_path);

    std::string script_content = read_file_content(in_progress_path);
    if (script_content.empty()) {
        logger->log(ERROR, "Script is empty or could not be read: " + in_progress_path);
        std::rename(in_progress_path.c_str(), (ACTIONS_FAILED_DIR + script_filename).c_str());
        return;
    }

    // Validate with QuantaEthos
    std::string validation_reason;
    if (!validate_with_ethos(script_content, validation_reason)) {
        logger->log(ERROR, "QuantaEthos validation failed: " + validation_reason);
        std::rename(in_progress_path.c_str(), (ACTIONS_FAILED_DIR + script_filename).c_str());
        std::string result_filename = "result-" + script_filename + ".json";
        std::string result_content = "{\"status\": \"failed\", \"reason\": \"Validation failed: " + validation_reason + "\"}";
        write_to_file(QUEUE_FAILED_DIR + result_filename, result_content);
        return;
    }
    logger->log(INFO, "QuantaEthos validation successful.");

    // Execute the script
    ScriptResult result = execute_script(in_progress_path);

    // Handle result
    std::string result_filename = "result-" + script_filename + ".json";
    std::string result_content;
    if (result.exit_code == 0) {
        logger->log(INFO, "Script executed successfully: " + script_filename);
        result_content = "{\"status\": \"completed\", \"exit_code\": 0, \"stdout\": \"" + result.stdout_output + "\"}";
        write_to_file(QUEUE_COMPLETED_DIR + result_filename, result_content);
        // Remove from in_progress on success
        std::remove(in_progress_path.c_str());
    } else {
        logger->log(ERROR, "Script execution failed with exit code " + std::to_string(result.exit_code));
        result_content = "{\"status\": \"failed\", \"exit_code\": " + std::to_string(result.exit_code) + ", \"stderr\": \"" + result.stderr_output + "\"}";
        write_to_file(QUEUE_FAILED_DIR + result_filename, result_content);
        // Move to failed on failure
        std::rename(in_progress_path.c_str(), (ACTIONS_FAILED_DIR + script_filename).c_str());
    }
}


/**
 * @brief Executes a script and captures its output.
 * @param script_path The full path to the script to execute.
 * @return A ScriptResult struct containing the exit code, stdout, and stderr.
 */
ScriptResult execute_script(const std::string& script_path) {
    logger->log(INFO, "Executing script: " + script_path);
    ScriptResult result;
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        logger->log(ERROR, "Failed to create pipes for script execution.");
        result.exit_code = -1;
        result.stderr_output = "Failed to create pipes.";
        return result;
    }

    pid_t pid = fork();
    if (pid == -1) {
        logger->log(ERROR, "Failed to fork process for script execution.");
        result.exit_code = -1;
        result.stderr_output = "Failed to fork.";
        return result;
    }

    if (pid == 0) { // --- Child Process ---
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        execl("/bin/sh", "sh", script_path.c_str(), nullptr);
        // If execl returns, it's an error
        perror("execl");
        _exit(127);
    } else { // --- Parent Process ---
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        int status;
        waitpid(pid, &status, 0);

        result.stdout_output = read_from_pipe(stdout_pipe[0]);
        result.stderr_output = read_from_pipe(stderr_pipe[0]);

        if (WIFEXITED(status)) {
            result.exit_code = WEXITSTATUS(status);
        } else {
            result.exit_code = -1; // Indicate abnormal termination
        }

        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
    }

    return result;
}

/**
 * @brief Reads the entire content of a file into a string.
 * @param path The path to the file.
 * @return The content of the file, or an empty string on failure.
 */
std::string read_file_content(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        logger->log(ERROR, "Failed to open file: " + path);
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}


/**
 * @brief Validates a script's content using the QuantaEthos validator.
 * @param script_content The content of the script to validate.
 * @param reason A string to be populated with the reason for failure.
 * @return True if validation is successful, false otherwise.
 */
bool validate_with_ethos(const std::string& script_content, std::string& reason) {
    logger->log(INFO, "Validating script with QuantaEthos...");
    // Escape double quotes in script content before passing to shell
    std::string escaped_content = script_content;
    size_t pos = 0;
    while ((pos = escaped_content.find("\"", pos)) != std::string::npos) {
        escaped_content.replace(pos, 1, "\\\"");
        pos += 2;
    }

    std::string command = ETHOS_VALIDATOR_PATH + " \"" + escaped_content + "\"";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        reason = "Failed to execute QuantaEthos validator.";
        logger->log(ERROR, reason);
        return false;
    }

    char buffer[1024];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);

    logger->log(DEBUG, "QuantaEthos response: " + result);

    // Simple JSON parsing to find the decision
    size_t decision_pos = result.find("\"decision\"");
    if (decision_pos != std::string::npos) {
        size_t colon_pos = result.find(":", decision_pos);
        size_t value_start = result.find("\"", colon_pos);
        if (value_start != std::string::npos) {
            size_t value_end = result.find("\"", value_start + 1);
            if (value_end != std::string::npos) {
                std::string decision = result.substr(value_start + 1, value_end - value_start - 1);
                if (decision == "approve") {
                    return true;
                }
            }
        }
    }

    // If decision is not "approve", extract the reason
    size_t reason_pos = result.find("\"reason\"");
    if (reason_pos != std::string::npos) {
        size_t colon_pos = result.find(":", reason_pos);
        size_t value_start = result.find("\"", colon_pos);
        if (value_start != std::string::npos) {
            size_t value_end = result.find("\"", value_start + 1);
            if (value_end != std::string::npos) {
                reason = result.substr(value_start + 1, value_end - value_start - 1);
                return false;
            }
        }
    }

    reason = "Could not parse decision or reason from QuantaEthos response.";
    return false;
}

/**
 * @brief Writes a string to a file.
 * @param path The path of the file to write to.
 * @param content The content to write to the file.
 */
void write_to_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        logger->log(ERROR, "Failed to open file for writing: " + path);
    }
}


/**
 * @brief Listens for the ESC key to set the global exit flag.
 */
void listen_for_exit() {
    set_terminal_raw(true);
    while (!exit_flag) {
        if (kbhit() == 27) { // 27 is the ASCII code for ESC
            exit_flag = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    set_terminal_raw(false);
}

// --- Function Implementations ---

/**
 * @brief Reads all available data from a non-blocking pipe.
 * @param fd The file descriptor of the pipe to read from.
 * @return The data read from the pipe as a string.
 */
std::string read_from_pipe(int fd) {
    std::string buffer;
    char read_buffer[256];
    ssize_t bytes_read;

    // Set pipe to non-blocking
    fcntl(fd, F_SETFL, O_NONBLOCK);

    while ((bytes_read = read(fd, read_buffer, sizeof(read_buffer) - 1)) > 0) {
        read_buffer[bytes_read] = '\0';
        buffer += read_buffer;
    }
    return buffer;
}

/**
 * @brief Sets the terminal to raw or cooked mode.
 * @param raw True for raw mode, false for cooked mode.
 */
void set_terminal_raw(bool raw) {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if (raw) {
        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;
    } else {
        tty.c_lflag |= ICANON;
        tty.c_lflag |= ECHO;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/**
 * @brief Checks if a key has been pressed.
 * @return The ASCII value of the key pressed, or -1 if no key was pressed.
 */
int kbhit() {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
        char c;
        if (read(STDIN_FILENO, &c, sizeof(c)) == 1) {
            return c;
        }
    }
    return -1;
}
