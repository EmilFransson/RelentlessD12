#pragma once

namespace Relentless
{
	template<typename T>
	class Broadcaster;

	using CallbackID = uint32_t;
	
	template<typename ReturnValue, typename... Args>
	class Broadcaster<ReturnValue(Args...)>
	{
	public:
		using CallbackType = std::function<ReturnValue(Args...)>;

		CallbackID Connect(CallbackType&& callback) noexcept
		{
			const CallbackID newID = m_NextID++;
			m_Callbacks[newID] = std::move(callback);
			return newID;
		}

		template<typename Object>
		void Connect(Object* pObject, ReturnValue(Object::* func)(Args...))
		{
			RLS_ASSERT(pObject, "Object pointer is invalid.");
			if (!pObject)
				return;

			auto boundCallback = [pObject, func](Args... args) -> ReturnValue { return (pObject->*func)(std::forward<Args>(args)...); };
			m_ObjectCallbacks[static_cast<void*>(pObject)].emplace_back(std::move(boundCallback));
		}

		void Detach(CallbackID id) noexcept
		{
			auto it = m_Callbacks.find(id);
			RLS_ASSERT(it != m_Callbacks.end(), "Callback ID is invalid.");

			if (it != m_Callbacks.end())
				m_Callbacks.erase(it);
		}

		template<typename Object>
		void Detach(Object* pObject) noexcept
		{
			RLS_ASSERT(pObject, "Object pointer is invalid.");
			if (!pObject)
				return;

			auto it = m_ObjectCallbacks.find(static_cast<void*>(pObject));
			RLS_ASSERT(it != m_ObjectCallbacks.end(), "Map entry does not exist.");

			if (it != m_ObjectCallbacks.end())
				m_ObjectCallbacks.erase(it);
		}

		void DetachAll() noexcept
		{
			m_Callbacks.clear();
			m_ObjectCallbacks.clear();
		}

		template<typename T = ReturnValue>
		auto operator()(Args... args) -> typename std::enable_if<std::is_same<T, void>::value>::type
		{
			for (auto& [id, callback] : m_Callbacks)
			{
				callback(std::forward<Args>(args)...);
			}
			for (auto& [obj, callbackList] : m_ObjectCallbacks)
			{
				for (auto& callback : callbackList)
				{
					callback(std::forward<Args>(args)...);
				}
			}
		}

		template<typename T = ReturnValue>
		auto operator()(Args... args) -> typename std::enable_if<!std::is_same<T, void>::value, std::vector<T>>::type
		{
			std::vector<T> results;
			for (auto& [id, callback] : m_Callbacks)
			{
				results.push_back(callback(std::forward<Args>(args)...));
			}
			for (auto& [obj, callbackList] : m_ObjectCallbacks)
			{
				for (auto& callback : callbackList)
				{
					results.push_back(callback(std::forward<Args>(args)...));
				}
			}

			return results;
		}

	private:
		std::unordered_map<CallbackID, CallbackType> m_Callbacks;
		std::unordered_map<void*, std::vector<CallbackType>> m_ObjectCallbacks;
		CallbackID m_NextID = 0u;
	};
}