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

        m_jobs.erase(it);

        for (auto& job : m_jobs)
        {
            std::erase(
                job.dependencies,
                id
            );
        }
    }

    void Scheduler::execute()
    {
        std::vector<JobID> completed;
        while (completed.size() < m_jobs.size())
        {
            for (auto& job : m_jobs)
            {
                bool ready = true;

                for (JobID dependency : job.dependencies)
                {
                    if (std::find(
                        completed.begin(),
                        completed.end(),
                        dependency
                    ) == completed.end())
                    {
                        ready = false;
                        break;
                    }
                }

                if (!ready)
                    continue;

                if (job.task() > 0)
                {
                    completed.push_back(job.id);

                    if (job.exit != nullptr)
                    {
                        job.exit();
                    }
                }
            }
        }
    }
}
