#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sys/inotify.h>

// Event types that the watcher can report
enum class FileEventType {
    CREATE,
    MODIFY,
    DELETE,
    UNKNOWN
};

// Structure to hold information about a file system event
struct FileEvent {
    FileEventType type;
    std::string path;
};

// A thread-safe queue for passing file events from the watcher thread
// to the main thread.
template<typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(value));
        cond.notify_one();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return false;
        }
        value = std::move(queue.front());
        queue.pop();
        return true;
    }

private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cond;
};

class FileWatcher {
public:
    FileWatcher(const std::vector<std::string>& directories, ThreadSafeQueue<FileEvent>& event_queue);
    ~FileWatcher();

    // Starts the file watching in a separate thread.
    void start();

    // Stops the file watching.
    void stop();

private:
    void watch_loop();

    std::vector<std::string> directories;
    ThreadSafeQueue<FileEvent>& event_queue;
    std::thread watcher_thread;
    std::atomic<bool> running;
    int inotify_fd;
    std::vector<int> watch_descriptors;
};

#endif // FILE_WATCHER_H
