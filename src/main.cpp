#include "../include/logger.h"
#include "../include/config.h"
#include "../include/file_watcher.h"
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
#include <fcntl.h>
#include <fstream>
#include <atomic>
#include <cstdio>
#include <csignal>
#include <sys/stat.h>

// --- Globals ---
Logger* logger = nullptr;
const Config* config = nullptr;
std::atomic<bool> exit_flag(false);

// --- Constants ---
const std::string EVENT_PIPE_PATH = "/tmp/prismquanta_events.pipe";
const std::string ACTIONS_PENDING_DIR = "actions/pending/";

// --- Function Prototypes ---
void set_terminal_raw(bool raw);
int kbhit();
void listen_for_exit();
pid_t launch_persistent_agent(const std::string& agent_path);
void cleanup();

/**
 * @brief Main entry point for the parent controller.
 */
int main(int argc, char* argv[]) {
    config = new Config("config.yaml");
    const AppConfig& appConfig = config->get();
    logger = new Logger(appConfig.log_file, appConfig.log_level);

    logger->log(INFO, "Parent controller started. Press ESC to exit.");

    unlink(EVENT_PIPE_PATH.c_str());
    if (mkfifo(EVENT_PIPE_PATH.c_str(), 0666) == -1) {
        logger->log(ERROR, "Failed to create named pipe: " + std::string(strerror(errno)));
        cleanup();
        return 1;
    }

    pid_t agent_pid = launch_persistent_agent(appConfig.agent_path);
    if (agent_pid == -1) {
        logger->log(ERROR, "Failed to launch Python agent. Shutting down.");
        cleanup();
        return 1;
    }
    logger->log(INFO, "Launched persistent agent with PID: " + std::to_string(agent_pid));

    ThreadSafeQueue<FileEvent> event_queue;
    std::vector<std::string> dirs_to_watch = {ACTIONS_PENDING_DIR, "src/"};
    FileWatcher watcher(dirs_to_watch, event_queue);
    watcher.start();

    std::thread exit_listener(listen_for_exit);

    logger->log(INFO, "Opening event pipe for writing...");
    int event_pipe_fd = open(EVENT_PIPE_PATH.c_str(), O_WRONLY);
    if (event_pipe_fd == -1) {
        logger->log(ERROR, "Failed to open named pipe for writing: " + std::string(strerror(errno)) + ". Shutting down.");
        kill(agent_pid, SIGTERM);
        cleanup();
        return 1;
    }
    logger->log(INFO, "Event pipe opened successfully.");

    while (!exit_flag) {
        FileEvent event;
        if (event_queue.try_pop(event)) {
            std::string event_str;
            switch(event.type) {
                case FileEventType::CREATE: event_str = "CREATE"; break;
                case FileEventType::MODIFY: event_str = "MODIFY"; break;
                case FileEventType::DELETE: event_str = "DELETE"; break;
                default: event_str = "UNKNOWN";
            }
            event_str += ":" + event.path + "\n";

            logger->log(DEBUG, "Publishing event: " + event_str);
            ssize_t bytes_written = write(event_pipe_fd, event_str.c_str(), event_str.length());
            if (bytes_written == -1) {
                logger->log(ERROR, "Failed to write to event pipe: " + std::string(strerror(errno)));
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    logger->log(INFO, "Exit flag set. Shutting down.");
    watcher.stop();
    exit_listener.join();

    close(event_pipe_fd);
    kill(agent_pid, SIGTERM);
    cleanup();

    return 0;
}

void cleanup() {
    if (logger) delete logger;
    logger = nullptr;
    if (config) delete config;
    config = nullptr;
    unlink(EVENT_PIPE_PATH.c_str());
}

pid_t launch_persistent_agent(const std::string& agent_path) {
    pid_t pid = fork();
    if (pid == -1) {
        if (logger) logger->log(ERROR, "Failed to fork for agent launch.");
        return -1;
    }

    if (pid == 0) { // Child Process
        execlp("python3", "python3", agent_path.c_str(), (char*)NULL);
        perror("execlp");
        _exit(127);
    }
    return pid;
}

void listen_for_exit() {
    set_terminal_raw(true);
    while (!exit_flag) {
        if (kbhit() == 27) {
            exit_flag = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    set_terminal_raw(false);
}

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
