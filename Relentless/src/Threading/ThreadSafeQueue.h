#pragma once

namespace Relentless
{
	template<typename DataType>
	class ThreadSafeQueue
	{
	public:
		explicit ThreadSafeQueue() noexcept {}
		ThreadSafeQueue(const ThreadSafeQueue& otherQueue) noexcept = delete;
		ThreadSafeQueue& operator=(const ThreadSafeQueue& otherQueue) noexcept = delete;

		void Push(DataType dataType) noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			m_Queue.push(std::move(dataType));
		}

		[[nodiscard]] bool TryPop(DataType& dataType) noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			if (m_Queue.empty())
				return false;

			dataType = std::move(m_Queue.front());
			m_Queue.pop();

			return true;
		}

		[[nodiscard]] bool Empty() noexcept
		{
			std::lock_guard<std::mutex> guard(m_Mutex);
			return m_Queue.empty();
		}
	private:
		std::queue<DataType> m_Queue;
		std::mutex m_Mutex;
	};
}