#include "../include/LavaEngine/Scheduler.hpp"

namespace LavaEngine
{
    JobID Scheduler::createJob(
        std::function<int()> task,
        std::function<void()> exit
    )
    {
        JobID id = m_nextID++;

        m_jobs.push_back(
            Job{
                .id = id,
                .task = std::move(task),
                .exit = std::move(exit)
            }
        );

        return id;
    }

    Scheduler::Scheduler(Scheduler&& other) noexcept
        : m_jobs(std::move(other.m_jobs)),
          m_nextID(other.m_nextID)
    {
        other.m_nextID = 0;
    }

    Scheduler& Scheduler::operator=(Scheduler&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_jobs = std::move(other.m_jobs);
        m_nextID = other.m_nextID;

        other.m_nextID = 0;

        return *this;
    }

    Job* Scheduler::findJob(JobID id)
    {
        for (auto& job : m_jobs)
        {
            if (job.id == id)
                return &job;
        }

        return nullptr;
    }

    void Scheduler::dependsOn(
        JobID id,
        JobID dependency
    )
    {
        if (id == dependency)
            throw std::runtime_error(
                "A job cannot depend on itself"
            );

        Job* job = findJob(id);

        if (!job)
            throw std::runtime_error(
                "Invalid JobID"
            );

        if (!findJob(dependency))
            throw std::runtime_error(
                "Invalid dependency JobID"
            );

        if (std::find(
            job->dependencies.begin(),
            job->dependencies.end(),
            dependency
        ) != job->dependencies.end())
        {
            return;
        }

        job->dependencies.push_back(dependency);
    }

    void Scheduler::destroyJob(JobID id)
    {
        auto it = std::find_if(
            m_jobs.begin(),
            m_jobs.end(),
            [id](const Job& job)
            {
                return job.id == id;
            }
        );

        if (it == m_jobs.end())
            return;

        // Exit.
        if (it->exit)
            it->exit();

        // Remove this job from dependency lists.
        for (auto& job : m_jobs)
        {
            std::erase(job.dependencies, id);
        }

        // Remove from completed jobs.
        std::erase(m_jobs_completed, id);

        // Finally destroy the job.
        m_jobs.erase(it);
    }


    void Scheduler::exitAll()
    {
        for (auto& job : m_jobs)
        {
            if (job.exit)
                job.exit();
        }

        m_jobs.clear();
        m_jobs_completed.clear();
    }


    void Scheduler::execute()
    {
        for (auto& job : m_jobs)
        {
            // A completed job doesn't execute again.
            if (std::find(
                    m_jobs_completed.begin(),
                    m_jobs_completed.end(),
                    job.id
                ) != m_jobs_completed.end())
            {
                continue;
            }

            bool ready = true;

            for (JobID dependency : job.dependencies)
            {
                if (std::find(
                        m_jobs_completed.begin(),
                        m_jobs_completed.end(),
                        dependency
                    ) == m_jobs_completed.end())
                {
                    ready = false;
                    break;
                }
            }

            if (!ready)
                continue;

            const int result = job.task();

            if (result > 0)
            {
                m_jobs_completed.push_back(job.id);

                if (job.exit)
                    job.exit();
            }
        }
    }

    void Scheduler::clear()
    {
        exitAll();

        m_jobs.clear();
        m_jobs_completed.clear();
    }
}
