#include "../include/file_watcher.h"
#include "../include/logger.h" // Assuming logger is accessible
#include <unistd.h>
#include <iostream>
#include <map>
#include <cstring> // For strerror
#include <cerrno>  // For errno

// Extern the global logger if it's defined in main.cpp
extern Logger* logger;

FileWatcher::FileWatcher(const std::vector<std::string>& dirs, ThreadSafeQueue<FileEvent>& queue)
    : directories(dirs), event_queue(queue), running(false), inotify_fd(-1) {
    inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd == -1) {
        if (logger) logger->log(ERROR, "Failed to initialize inotify");
    }
}

FileWatcher::~FileWatcher() {
    stop();
    if (inotify_fd != -1) {
        close(inotify_fd);
    }
}

void FileWatcher::start() {
    if (inotify_fd == -1) {
        if (logger) logger->log(ERROR, "Cannot start FileWatcher, inotify not initialized.");
        return;
    }
    running = true;
    watcher_thread = std::thread(&FileWatcher::watch_loop, this);
}

void FileWatcher::stop() {
    running = false;
    if (watcher_thread.joinable()) {
        watcher_thread.join();
    }
}

void FileWatcher::watch_loop() {
    std::map<int, std::string> wd_to_path;

    for (const auto& dir : directories) {
        int wd = inotify_add_watch(inotify_fd, dir.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE);
        if (wd == -1) {
            if (logger) logger->log(ERROR, "Cannot watch '" + dir + "': " + strerror(errno));
        } else {
            watch_descriptors.push_back(wd);
            wd_to_path[wd] = dir;
            if (logger) logger->log(INFO, "Started watching directory: " + dir);
        }
    }

    const size_t event_buf_len = 1024 * (sizeof(struct inotify_event) + 16);
    char buffer[event_buf_len];

    while (running) {
        int length = read(inotify_fd, buffer, event_buf_len);
        if (length < 0) {
            // If non-blocking, EAGAIN is not an error, just means no events.
            if (errno != EAGAIN) {
                 if (logger) logger->log(ERROR, "Error reading from inotify fd: " + std::string(strerror(errno)));
            }
             // Wait a bit before trying again
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            if (event->len) {
                FileEvent fe;
                std::string dir_path = wd_to_path[event->wd];
                if (dir_path.back() == '/') {
                    fe.path = dir_path + event->name;
                } else {
                    fe.path = dir_path + "/" + event->name;
                }

                if (event->mask & IN_CREATE) {
                    fe.type = FileEventType::CREATE;
                } else if (event->mask & IN_MODIFY) {
                    fe.type = FileEventType::MODIFY;
                } else if (event->mask & IN_DELETE) {
                    fe.type = FileEventType::DELETE;
                } else {
                    fe.type = FileEventType::UNKNOWN;
                }

                if (fe.type != FileEventType::UNKNOWN) {
                    event_queue.push(fe);
                }
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }

    // Clean up watch descriptors
    for (int wd : watch_descriptors) {
        inotify_rm_watch(inotify_fd, wd);
    }
    watch_descriptors.clear();
     if (logger) logger->log(INFO, "File watcher loop stopped.");
}
