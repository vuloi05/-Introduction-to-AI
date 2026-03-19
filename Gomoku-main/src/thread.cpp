#include "thread.hpp"

#include <cassert>

namespace Gomoku
{
    Thread::Thread() : running_{false}, busy_{false} {}

    Thread::~Thread() { stop(); }

    void Thread::start()
    {
        std::lock_guard lock(mutex_);
        if (running_)
            return;
        running_ = true;
        thread_ = std::thread(&Thread::loop, this);
    }

    void Thread::stop()
    {
        {
            std::lock_guard lock(mutex_);
            if (!running_)
                return;
            running_ = false;
        }
        cv_.notify_all();
        if (thread_.joinable())
            thread_.join();
    }

    void Thread::assign_job(std::function<void()> job)
    {
        if (!job)
            return;
        std::unique_lock lock(mutex_);
        while (busy_)
            cv_.wait(lock);
        job_ = std::move(job);
        busy_ = true;
        cv_.notify_all();
    }

    void Thread::wait()
    {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !busy_; });
    }

    void Thread::loop()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this] { return (busy_ && job_) || !running_; });
                if (!running_ && !busy_)
                    break;
                if (job_)
                    job = std::move(job_);
            }
            if (job)
                job();
            {
                std::lock_guard lock(mutex_);
                busy_ = false;
                cv_.notify_all();
            }
        }
    }

    ThreadPool::ThreadPool(const std::size_t num_threads) { resize(num_threads); }

    ThreadPool::~ThreadPool() { threads_.clear(); }

    void ThreadPool::resize(const std::size_t new_size)
    {
        const std::size_t old_size = size();
        if (new_size == old_size)
            return;
        if (new_size > old_size)
        {
            for (std::size_t i = old_size; i < new_size; ++i)
            {
                auto thread = std::make_unique<Thread>();
                thread->start();
                threads_.emplace_back(std::move(thread));
            }
        }
        else
            for (std::size_t i = new_size; i < old_size; ++i)
                threads_.pop_back();
    }

    void ThreadPool::assign_job(const std::size_t thread_index, std::function<void()> job) const
    {
        assert(thread_index < size());
        threads_[thread_index]->assign_job(std::move(job));
    }

    void ThreadPool::wait_for_all() const
    {
        for (const auto& thread : threads_)
            thread->wait();
    }
}
