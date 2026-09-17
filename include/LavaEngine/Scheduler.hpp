#pragma once
#include <chrono>
#include <functional>
#include <memory>

namespace LavaEngine
{
    using JobID = uint32_t;

    /**
     * @brief One schedulable unit producing an int result, owned by a Scheduler.
     * @detail A Job runs `task` until it returns > 0 (complete); 0 or a
     * negative value keeps the job scheduled for the next execute(). `exit`
     * runs when the job completes or is destroyed. `dependencies` gate
     * execution.
     * @note Ownership: Owned by the owning Scheduler (std::vector<Job>).
     * `task`/`exit` may capture what the job needs, but captured references
     * must live at least until the job is destroyed. The Scheduler is
     * drained before containers/modules are destroyed, so jobs must not
     * capture Container/Module/Resource that could die earlier.
     * @example
     * @code
     * JobID id = scheduler.createJob([&] { doWork(); return 1; });
     * scheduler.dependsOn(id, otherJob);
     * @endcode
     */
    struct Job
    {
        JobID id;
        std::function<int()> task;
        std::function<void()> exit;
        std::vector<JobID> dependencies;
        std::chrono::microseconds duration;
    };

    /**
     * @brief Owns and advances a set of Jobs with dependency ordering.
     * @detail execute() runs every ready, incomplete job once per call and
     * records each job's duration; completion is decided by the job's return
     * value.
     * @note Ownership: Owned by `Application` (m_scheduler, by value). It
     * uniquely owns every Job. Application::unloadGame() drains it (exitAll)
     * before destroying frameworks/containers. Pointers from findJob() are
     * invalidated by createJob()/destroyJob() and must not be held across
     * those calls.
     * @example
     * @code
     * Scheduler scheduler;
     * scheduler.createJob([] { return 1; });
     * while (scheduler.completedJobCount() < scheduler.jobCount()) scheduler.execute();
     * @endcode
     */
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

        [[nodiscard]] std::size_t jobCount() const { return m_jobs.size(); }
        [[nodiscard]] std::vector<Job> jobs() const { return m_jobs; }
        [[nodiscard]] std::size_t completedJobCount() const { return m_jobs_completed.size(); }

    private:
        friend class Application;
        std::vector<Job> m_jobs;
        std::vector<JobID> m_jobs_completed;
        JobID m_nextID = 0;
    };
}