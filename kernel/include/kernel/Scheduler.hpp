#pragma once
#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace kernel {

// Task priority
enum class TaskPriority {
    LOW,
    NORMAL,
    HIGH,
    REALTIME
};

// Processing task
struct Task {
    std::function<void()> function;
    TaskPriority priority = TaskPriority::NORMAL;
    int task_id = 0;
};

// Scheduler - manages task execution
class Scheduler {
public:
    Scheduler(size_t num_threads = 4);
    ~Scheduler();
    
    // Start/stop
    void start();
    void stop();
    
    // Submit task
    int submit(std::function<void()> task, TaskPriority priority = TaskPriority::NORMAL);
    
    // Wait for task completion
    bool wait_for_task(int task_id, int timeout_ms = -1);
    
    // Statistics
    size_t get_pending_tasks() const;
    size_t get_completed_tasks() const;

private:
    void worker_thread();
    
    size_t num_threads_;
    std::vector<std::thread> workers_;
    
    std::queue<Task> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
    std::atomic<bool> running_{false};
    std::atomic<int> next_task_id_{0};
    std::atomic<size_t> completed_tasks_{0};
};

} // namespace kernel
