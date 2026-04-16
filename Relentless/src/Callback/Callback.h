#pragma once
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace Relentless
{
	template<typename Signature>
	class Callback;

	template<typename RetVal, typename... Args>
	class Callback<RetVal(Args...)>
	{
	public:
		Callback() noexcept = default;

		template<typename Func, typename = std::enable_if_t<!std::is_same_v<std::decay_t<Func>, Callback>>>
		Callback(Func&& func)
			: m_CallbackFunc(std::make_shared<std::function<RetVal(Args...)>>(std::forward<Func>(func)))
		{
		}

		template<typename Func,
			typename = std::enable_if_t<!std::is_same_v<std::decay_t<Func>, Callback>>>
		Callback& operator=(Func&& func)
		{
			m_CallbackFunc = std::make_shared<std::function<RetVal(Args...)>>(std::forward<Func>(func));
			return *this;
		}

		Callback(const Callback&) noexcept = default;
		Callback(Callback&&) noexcept = default;
		Callback& operator=(const Callback&) noexcept = default;
		Callback& operator=(Callback&&) noexcept = default;

		RetVal operator()(Args... args) const
		{
			if (!m_CallbackFunc)
			{
				RLS_ASSERT(false, "[Callback]: No callback function set.");

				if constexpr (std::is_void_v<RetVal>)
					return;
				else
					std::terminate();
			}

			if constexpr (std::is_void_v<RetVal>)
			{
				(*m_CallbackFunc)(std::forward<Args>(args)...);
				return;
			}
			else
				return (*m_CallbackFunc)(std::forward<Args>(args)...);
		}

		template<class Obj, class MemFn>
		requires std::is_invocable_r_v<RetVal, MemFn, Obj*, Args...>
		static Callback Bind(Obj* aInstance, MemFn aMethod)
		{
			return Callback([aInstance, aMethod](Args... args) -> RetVal
				{
					return std::invoke(aMethod, aInstance, std::forward<Args>(args)...);
				});
		}

		template<class Func>
		requires std::is_invocable_r_v<RetVal, Func&, Args...>
		static Callback Bind(Func&& aFunction)
		{
			return Callback(std::forward<Func>(aFunction));
		}

		[[nodiscard]] bool IsSet() const noexcept { return (bool)m_CallbackFunc; }
		void Clear() noexcept { m_CallbackFunc.reset(); }

		void ExecuteIfSet(Args... args) const
		{
			if (m_CallbackFunc)
				(*m_CallbackFunc)(std::forward<Args>(args)...);
		}

	private:
		std::shared_ptr<std::function<RetVal(Args...)>> m_CallbackFunc;
	};
}