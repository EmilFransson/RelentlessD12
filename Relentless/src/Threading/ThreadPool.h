#pragma once
#include "ThreadSafeQueue.h"

namespace Relentless
{
	class ThreadPool
	{
	public:
		explicit ThreadPool(uint32_t threadCount = std::thread::hardware_concurrency() - 1)
		{
			try
			{
				for (uint32_t i{ 0u }; i < threadCount; ++i)
				{
					m_Threads.push_back(std::jthread(&ThreadPool::WorkerThread, this));
				}
			}
			catch (...)
			{
				m_Done = true;
				throw;
			}
		}

		~ThreadPool() noexcept
		{
			m_Done = true;
			m_Condition.notify_all();
			for (auto& thread : m_Threads)
			{
				if (thread.joinable())
					thread.join();
			}
		}

		template<class FunctionType, class... Args>
		inline std::future<typename std::invoke_result<FunctionType, Args...>::type> Submit(FunctionType&& function, Args&&... args) 
		{
			using returnType = typename std::invoke_result<FunctionType, Args...>::type;
			auto task = std::make_shared<std::packaged_task<returnType()>>(
				std::bind(std::forward<FunctionType>(function), std::forward<Args>(args)...));

			std::future<returnType> result = task->get_future();
			m_WorkQueue.Push([task]() { (*task)(); });

			m_Condition.notify_one();
			return result;
		}
		[[nodiscard]] size_t GetThreadCount() const noexcept { return m_Threads.size(); }
	private:
		void WorkerThread()
		{
			while (!m_Done)
			{
				{
					std::unique_lock<std::mutex> lock(m_Mutex);
					m_Condition.wait(lock, [this]()
						{
							return m_Done || !m_WorkQueue.Empty();
						});
					if (m_Done && m_WorkQueue.Empty())
						break;
				}
				std::function<void()> task;
				if (m_WorkQueue.TryPop(task))
				{
					task();
				}
			}
		}
	private:
		std::mutex m_Mutex;
		std::condition_variable m_Condition;
		std::atomic_bool m_Done = false;
		ThreadSafeQueue<std::function<void()>> m_WorkQueue;
		std::vector<std::jthread> m_Threads;
	};
}