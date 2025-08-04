#include <thread>
#include <chrono>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <cstring>
#include <sys/ioctl.h>

Logger logger("parent.log", DEBUG);  // Adjust log level here

// ... (set_terminal_raw, kbhit, is_in_time_window unchanged)

int launch_agent() {
    logger.log(INFO, "Launching agent process...");
    pid_t pid = fork();
    if (pid == 0) {
        execl("/usr/bin/python3", "python3", "/path/to/agent.py", nullptr);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            logger.log(INFO, "Agent exited with code " + std::to_string(code));
            return code;
        } else {
            logger.log(ERROR, "Agent terminated abnormally");
            return -1;
        }
    }
    logger.log(ERROR, "Failed to fork process");
    return -1;
}

int main(int argc, char* argv[]) {
    logger.log(INFO, "Parent started");

    if (!is_in_time_window()) {
        logger.log(INFO, "Not in time window, exiting");
        return 0;
    }

    int exit_code = launch_agent();

    logger.log(INFO, "Waiting up to 15 seconds for ESC press to exit early");
    set_terminal_raw(true);
    auto start = std::chrono::steady_clock::now();
    while (true) {
        int key = kbhit();
        if (key == 27) { // ESC
            logger.log(INFO, "ESC pressed, exiting early");
            break;
        }
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::seconds(15)) {
            logger.log(INFO, "Timeout reached, exiting");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    set_terminal_raw(false);

    logger.log(INFO, "Parent exiting");
    return 0;
}
