#pragma once
#include <functional>
#include <memory>

namespace LavaEngine
{
    using JobID = uint32_t;

    struct Job
    {
        JobID id;
        std::function<int()> task;
        std::function<void()> exit;
        std::vector<JobID> dependencies;
    };

    class Scheduler
    {
    public:
        Scheduler() = default;
        ~Scheduler() = default;

        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;

        Scheduler(Scheduler&& other) noexcept;
        Scheduler& operator=(Scheduler&& other) noexcept;

        JobID createJob(std::function<int()> task, std::function<void()> exit = nullptr);
        Job* findJob(JobID id);

        void destroyJob(JobID id);
        void execute();

        void dependsOn(JobID id, JobID dependency);

    private:
        std::vector<Job> m_jobs;
        JobID m_nextID = 0;
    };
}
