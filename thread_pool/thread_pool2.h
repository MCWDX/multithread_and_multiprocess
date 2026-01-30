#pragma once

#include <thread>
#include <condition_variable>
#include <future>
#include <functional>
#include <vector>
#include <queue>
#include <stdexcept>
#include <atomic>
#include <iostream>

class ThreadPool2 {
public:
    ThreadPool2() = delete;
    explicit ThreadPool2(uint32_t thread_count) {
        thread_count_.store(thread_count);
        launched_.store(false);
        shutdown_.store(false);
    }

    ~ThreadPool2() {
        shutdown();
    }

    /**
     * @brief 启动线程池
     * @return 返回该次操作是否成功启动线程池
     */
    bool launch() {
        if (launched_.load()) {
            std::cout << "Thread Pool already launched" << std::endl;
            return false;
        }
        if (shutdown_.load()) {
            std::cout << "Thread Pool already shutdown" << std::endl;
            return false;
        }
        for (uint32_t i = 0; i < thread_count_.load(); i++) {
            addThread();
            std::cout << "Worker " << i << " ready" << std::endl;
        }
        launched_.store(true);
        return true;
    }

    /**
     * @param 关闭线程池
     * @return 返回该次操作是否关闭了线程池
     */
    bool shutdown() {
        if (!launched_.load()) {
            std::cout << "Thread Pool is not launched yet" << std::endl;
            return false;
        }
        if (shutdown_.load()) {
            std::cout << "Thread Pool already shutdown" << std::endl;
            return false;
        }

        shutdown_.store(true);
        tasks_cv_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
            std::cout << "Worker shutdown" << std::endl;
        }
        return true;
    }

    /**
     * @brief 往线程池添加任务
     * @param func 可调用的函数
     * @param args func所需的参数
     * @return 返回std::future<func返回类型>实例，如要获取func的返回结果，则对返回值调用get()函数
     */
    template <typename F, typename... Args>
    auto addTask(F&& func, Args&&... args) -> std::future<std::result_of_t<F(Args...)>> {
        if (!launched_.load()) {
            throw std::runtime_error("Thread Pool is not launched yet");
        }
        if (shutdown_.load()) {
            throw std::runtime_error("Thread Pool already shutdown");
        }

        using ReturnType = std::result_of_t<F(Args...)>;
        std::packaged_task<ReturnType()> task(std::bind(std::forward<F>(func), std::forward<Args>(args)...));


        auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(std::move(task));

        std::future<ReturnType> res = task_ptr->get_future();
        {
            std::unique_lock<std::mutex> lock(tasks_mutex_);
            tasks_.emplace([task_ptr]() { (*task_ptr)(); });
        }
        tasks_cv_.notify_all();
        return res;
    }

    /**
     * @brief 往线程池提交无返回值的任务
     * @param func 返回值为void类型的函数
     * @param args func所需参数
     * @return 任务是否正常提交到线程池
     */
    template<typename F, typename... Args>
    bool addTaskNoRet(F&& func, Args&&... args) {
        if (!launched_.load()) {
            std::cout << "Thread Pool is not launched yet" << std::endl;
            return false;
        }
        if (shutdown_.load()) {
            std::cout << "Thread Pool already shutdown" << std::endl;
            return false;
        }

        auto task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(tasks_mutex_);
            tasks_.emplace(std::move(task));
        }

        tasks_cv_.notify_all();
    }
    
private:
    /**
     * @brief 往线程池添加给定数量的子线程
     */
    void addThread() {
        auto func = [this]() {
            for (;;) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(tasks_mutex_);
                    tasks_cv_.wait(lock, [this]() { return shutdown_.load() || !tasks_.empty(); });

                    if (tasks_.empty()) {
                        if (shutdown_.load()) {
                            break;
                        } else {
                            continue;
                        }
                    }
                    
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        };

        workers_.emplace_back(std::move(func));
    }

    using Task = std::function<void()>;
    std::vector<std::thread> workers_;

    std::queue<Task> tasks_;
    std::condition_variable tasks_cv_;
    std::mutex tasks_mutex_;

    std::atomic<uint32_t> thread_count_;
    std::atomic<bool> launched_;
    std::atomic<bool> shutdown_;
};