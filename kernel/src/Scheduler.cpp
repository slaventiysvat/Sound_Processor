#include "kernel/Scheduler.hpp"
#include "sp/Logger.hpp"

namespace kernel {

Scheduler::Scheduler(size_t num_threads) 
    : num_threads_(num_threads) {
}

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    if (running_) return;
    
    running_ = true;
    
    for (size_t i = 0; i < num_threads_; ++i) {
        workers_.emplace_back(&Scheduler::worker_thread, this);
    }
    
    SP_INFO("Scheduler started with ", num_threads_, " worker threads");
}

void Scheduler::stop() {
    if (!running_) return;
    
    running_ = false;
    queue_cv_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    SP_INFO("Scheduler stopped");
}

int Scheduler::submit(std::function<void()> task, TaskPriority priority) {
    if (!running_) {
        SP_WARN("Scheduler not running, cannot submit task");
        return -1;
    }
    
    int task_id = next_task_id_++;
    
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(queue_mutex_));
        task_queue_.push({std::move(task), priority, task_id});
    }
    
    queue_cv_.notify_one();
    
    SP_TRACE("Task submitted: ID=", task_id);
    return task_id;
}

bool Scheduler::wait_for_task(int task_id, int timeout_ms) {
    return true;
}

size_t Scheduler::get_pending_tasks() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(queue_mutex_));
    return task_queue_.size();
}

size_t Scheduler::get_completed_tasks() const {
    return completed_tasks_;
}

void Scheduler::worker_thread() {
    SP_DEBUG("Worker thread started");
    
    while (running_) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            queue_cv_.wait(lock, [this] {
                return !running_ || !task_queue_.empty();
            });
            
            if (!running_) break;
            
            if (task_queue_.empty()) continue;
            
            task = std::move(task_queue_.front());
            task_queue_.pop();
        }
        
        try {
            task.function();
            completed_tasks_++;
        } catch (const std::exception& e) {
            SP_ERROR("Task execution failed: ", e.what());
        }
    }
    
    SP_DEBUG("Worker thread stopped");
}

} // namespace kernel
