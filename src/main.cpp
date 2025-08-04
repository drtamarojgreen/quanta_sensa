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
#include <fcntl.h> // For fcntl

// --- Globals ---
Logger* logger = nullptr;
const Config* config = nullptr;

// --- Function Prototypes ---
void set_terminal_raw(bool raw);
int kbhit();
bool is_in_time_window();
std::string read_from_pipe(int fd);

/**
 * @brief Launches the Python agent, capturing its stdout and stderr.
 */
int launch_agent() {
    logger->log(INFO, "Setting up pipes for agent communication...");
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
        logger->log(ERROR, "Failed to create pipes.");
        return -1;
    }

    logger->log(INFO, "Launching agent process...");
    pid_t pid = fork();

    if (pid == -1) {
        logger->log(ERROR, "Failed to fork process.");
        return -1;
    }

    if (pid == 0) { // --- Child Process ---
        // Redirect stdout and stderr
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        // Close unused pipe ends
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        const std::string& agent_path = config->get().agent_path;
        execl("/usr/bin/python3", "python3", agent_path.c_str(), nullptr);
        logger->log(ERROR, "Failed to execute agent script at: " + agent_path);
        _exit(127);
    } else { // --- Parent Process ---
        // Close unused write ends of the pipes
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        int status;
        waitpid(pid, &status, 0);

        // Read stdout and stderr from the pipes
        std::string agent_stdout = read_from_pipe(stdout_pipe[0]);
        std::string agent_stderr = read_from_pipe(stderr_pipe[0]);

        if (!agent_stdout.empty()) {
            logger->log(INFO, "Agent stdout:\n" + agent_stdout);
        }
        if (!agent_stderr.empty()) {
            logger->log(ERROR, "Agent stderr:\n" + agent_stderr);
        }

        // Close read ends
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            logger->log(INFO, "Agent exited with code " + std::to_string(exit_code));
            return exit_code;
        } else {
            logger->log(ERROR, "Agent terminated abnormally.");
            return -1;
        }
    }
}

/**
 * @brief Main entry point for the parent controller.
 */
int main(int argc, char* argv[]) {
    config = new Config("config.yaml");
    const AppConfig& appConfig = config->get();
    logger = new Logger(appConfig.log_file, appConfig.log_level);

    logger->log(INFO, "Parent controller started.");

    if (!is_in_time_window()) {
        logger->log(INFO, "Not within the allowed time window. Exiting.");
        delete logger;
        delete config;
        return 0;
    }

    int exit_code = launch_agent();
    logger->log(DEBUG, "Agent launch completed with exit code: " + std::to_string(exit_code));

    int wait_seconds = appConfig.post_action_wait_seconds;
    logger->log(INFO, "Waiting up to " + std::to_string(wait_seconds) + " seconds for ESC press...");
    set_terminal_raw(true);
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        if (kbhit() == 27) {
            logger->log(INFO, "ESC key pressed. Exiting early.");
            break;
        }
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed >= std::chrono::seconds(wait_seconds)) {
            logger->log(INFO, "Timeout reached. Exiting.");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    set_terminal_raw(false);

    logger->log(INFO, "Parent controller exiting.");
    delete logger;
    delete config;
    return 0;
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

void set_terminal_raw(bool raw) { /* ... implementation ... */ }
int kbhit() { /* ... implementation ... */ return -1; }

bool is_in_time_window() {
    if (!config || !logger) return false;
    const auto& schedule = config->get().schedule;
    if (schedule.empty()) {
        logger->log(DEBUG, "Schedule is empty, allowing execution.");
        return true;
    }
    auto now = std::time(nullptr);
    auto local_time = *std::localtime(&now);
    int current_hour = local_time.tm_hour;
    for (const auto& window : schedule) {
        if (current_hour >= window.start_hour && current_hour <= window.end_hour) {
            logger->log(INFO, "Current time is within a scheduled window.");
            return true;
        }
    }
    logger->log(INFO, "Current time is outside of all scheduled windows.");
    return false;
}
