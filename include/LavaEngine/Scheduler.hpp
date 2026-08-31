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
        void exitAll();
        void execute();
        void clear();

        void dependsOn(JobID id, JobID dependency);

    private:
        friend class Application;
        std::vector<Job> m_jobs;
        std::vector<JobID> m_jobs_completed;
        JobID m_nextID = 0;
    };
}
