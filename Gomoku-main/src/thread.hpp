#ifndef GOMOKU_THREAD_HPP
#define GOMOKU_THREAD_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace Gomoku
{
    class Thread
    {
    public:
        Thread();

        Thread(const Thread&) = delete;
        Thread(Thread&&) = delete;
        Thread& operator=(const Thread&) = delete;
        Thread& operator=(Thread&&) = delete;

        ~Thread();

        void start();
        void stop();

        void assign_job(std::function<void()> job);
        void wait();

    private:
        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable cv_;

        std::function<void()> job_;

        bool running_;
        bool busy_;

        void loop();
    };

    class ThreadPool
    {
    public:
        explicit ThreadPool(std::size_t num_threads = 0);

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        ThreadPool(ThreadPool&&) = default;
        ThreadPool& operator=(ThreadPool&&) = default;

        ~ThreadPool();

        void resize(std::size_t new_size);
        void assign_job(std::size_t thread_index, std::function<void()> job) const;
        void wait_for_all() const;

        [[nodiscard]] std::size_t size() const;
    private:
        std::vector<std::unique_ptr<Thread>> threads_;
    };

    inline std::size_t ThreadPool::size() const { return threads_.size(); }
}

#endif //GOMOKU_THREAD_HPP
