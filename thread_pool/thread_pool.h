#pragma once

#include <functional>
#include <vector>
#include <queue>
#include <future>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <stdexcept>

class ThreadPool {
public:
    ThreadPool(uint32_t thread_count);
    ~ThreadPool();

    //禁止复制
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    bool start();

    void stop();

    template<typename F, typename... Args>
    bool addTask(F&& func, Args&&... args);

    template<typename Func, typename... Args>
    auto addTaskWithRet(Func&& func, Args&&... args) -> std::shared_ptr<std::future<std::result_of_t<Func(Args...)>>>;
    
private:
    using Task = std::function<void()>;
    using ThreadPtr = std::shared_ptr<std::thread>;
    struct ThreadInfo {
        ThreadInfo() = default;
        ~ThreadInfo();

        ThreadPtr ptr{nullptr};
    };

    using ThreadInfoPtr = std::shared_ptr<ThreadInfo>;

    void addThread();    

    std::vector<ThreadInfoPtr> threads_;
    std::queue<Task> task_queue_;

    std::mutex task_q_mutex_;
    std::condition_variable task_q_cv_;

    std::atomic<uint32_t> thread_count_;
    std::atomic<bool> is_available_;
    std::atomic<bool> is_shutdown_;
};

ThreadPool::ThreadInfo::~ThreadInfo() {
    if (ptr->joinable()) {
        ptr->join();
    }
    std::cout << "thread joined" << std::endl;
}

ThreadPool::ThreadPool(uint32_t thread_count) {
    thread_count_.store(thread_count);
    is_available_.store(true);
    is_shutdown_.store(false);
}

ThreadPool::~ThreadPool() {
    ThreadPool::stop();
}

bool ThreadPool::start() {
    //线程池已经start过，或者已经关闭了
    if (!is_available_.load() || is_shutdown_.load()) {
        return false;
    }

    //在启动/重新启动的时候将线程加回来
    for (uint32_t i = 0; i < thread_count_.load(); i++) {
        addThread();
        std::cout << "added thread " << i << std::endl;
    }

    is_available_.store(true);
    return true;
}

void ThreadPool::stop() {
    //本来就是关闭状态，不用stop
    if (is_available_.load()) {
        is_shutdown_.store(true);
    
        //唤醒所有线程让他们break
        task_q_cv_.notify_all();
        is_available_.store(false);
    }
    {
        std::unique_lock<std::mutex> lock(task_q_mutex_);
        std::cout << "At the time of stopping the thread pool,";
        std::cout << "there were " << task_queue_.size() << " tasks in the queue" << std::endl;
    }
    //清空当前线程
    threads_.clear();
}

template<typename F, typename... Args>
bool ThreadPool::addTask(F&& func, Args&&... args) {
    //如果线程池已关闭，或者没开始，返回false代表加入任务失败
    if (is_shutdown_.load() || !is_available_.load()) {
        return false;
    }
    // auto task = [&func, &args...] () { std::bind(func, std::forward<Args>(args)...); };
    // {
    //     std::unique_lock<std::mutex> lock(task_q_mutex_);
    //     task_queue_.push(task);
    // }
    auto task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);
    {
        //防御性编程，参考自训练营“线程池”文档的评论。
        if (is_shutdown_.load()) {
            throw std::runtime_error("");
        }
        std::unique_lock<std::mutex> lock(task_q_mutex_);
        task_queue_.push([task] () { task(); });
    }

    //加入新task后唤醒线程去执行
    task_q_cv_.notify_one();
    return true;
}

//生成线程并加入到线程库
void ThreadPool::addThread() {
    ThreadInfoPtr ptr = std::make_shared<ThreadInfo>();
    auto func = [this] () {
        for(;;) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(task_q_mutex_);
                task_q_cv_.wait(lock, [this] () { return !task_queue_.empty() || is_shutdown_.load(); });
                
                //线程池已关闭，该break了
                if (is_shutdown_.load()) {
                    break;
                }

                //等待条件里面已有判断队列是否空，不用再次判断

                //取出新的任务
                task = task_queue_.front();
                task_queue_.pop();
                
                //离开作用域，释放锁
            }
            //执行task
            task();
        }
    };


    //将线程加入到线程库中
    ptr->ptr = std::make_shared<std::thread>(std::move(func));
    threads_.emplace_back(std::move(ptr));
}


template<typename Func, typename... Args>
auto ThreadPool::addTaskWithRet(Func&& func, Args&&... args) -> std::shared_ptr<std::future<std::result_of_t<Func(Args...)>>> {
    if (!is_available_.load()) {
        std::cout << "Thread pool not launched yet" << std::endl;
        return nullptr;
    }
    if (is_shutdown_.load()) {
        std::cout << "Thread pool already shutdown" << std::endl;
        return nullptr;
    }
    using ReturnType = std::result_of_t<Func(Args...)>;
    auto task = 
        std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

    std::future<ReturnType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(task_q_mutex_);
        task_queue_.emplace([task]() { (*task)(); });
    }
    task_q_cv_.notify_all();
    return std::make_shared<std::future<std::result_of_t<Func(Args...)>>>(std::move(res));
}