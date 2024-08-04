#pragma once

#include <condition_variable> 
#include <future> 
#include <mutex> 
#include <queue> 
#include <thread>
#include <functional>
#include <type_traits>

class JobPool 
{
public:
    // // Constructor to creates a thread pool with given 
    // number of threads 
    JobPool(size_t num_threads = std::thread::hardware_concurrency())
    {

        // Creating worker threads 
        for (size_t i = 0; i < num_threads; ++i) {
            mThreads.emplace_back([this] {
                while (true) {
                    std::move_only_function<void()> task;
                    // The reason for putting the below code 
                    // here is to unlock the queue before 
                    // executing the task so that other 
                    // threads can perform enqueue tasks 
                    {
                        // Locking the queue so that data 
                        // can be shared safely 
                        std::unique_lock<std::mutex> lock(mQueueMutex);

                        // Waiting until there is a task to 
                        // execute or the pool is stopped 
                        mCV.wait(lock, [this] {
                            return !mTasks.empty() || mStop;
                            });

                        // exit the thread in case the pool 
                        // is stopped and there are no tasks 
                        if (mStop && mTasks.empty()) {
                            return;
                        }

                        // Get the next task from the queue 
                        task = std::move(mTasks.front());
                        mTasks.pop();
                    }

                    task();
                }
                });
        }
    }

    // Destructor to stop the thread pool 
    ~JobPool()
    {
        {
            // Lock the queue to update the stop flag safely 
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mStop = true;
        }

        // Notify all threads 
        mCV.notify_all();

        // Joining all worker threads to ensure they have 
        // completed their tasks 
        for (auto& thread : mThreads) {
            thread.join();
        }
    }

    // Enqueue task for execution by the thread pool 
    template<typename Function, typename... Args> requires std::invocable<Function, Args...>
    auto Queue(Function&& f, Args&&... args)
    {
        using ReturnType = std::invoke_result_t<Function, Args&&...>;
        std::promise<ReturnType> returnPromise;

        auto future = returnPromise.get_future();

        auto task = [job = std::move(f), ret = std::move(returnPromise), ...args = std::forward<Args>(args)]() mutable {
            ret.set_value(std::invoke(job, args...));
        };

        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mTasks.emplace(std::move(task));
        }
        mCV.notify_one();

        return future;
    }

private:
    // Vector to store worker threads 
    std::vector<std::thread> mThreads;

    // Queue of tasks 
    std::queue<std::move_only_function<void()> > mTasks;

    // Mutex to synchronize access to shared data 
    std::mutex mQueueMutex;

    // Condition variable to signal changes in the state of 
    // the tasks queue 
    std::condition_variable mCV;

    // Flag to indicate whether the thread pool should stop 
    // or not 
    bool mStop = false;
};