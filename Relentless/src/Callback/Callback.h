#pragma once

namespace Relentless
{
	template<typename Signature>
	class Callback;

	template<typename RetVal, typename... Args>
	class Callback<RetVal(Args...)>
	{
	public:
		Callback() noexcept = default;
		
		template<typename Func, typename = typename std::enable_if<
			!std::is_same<typename std::decay<Func>::type, Callback>::value>::type>
		Callback(Func&& func)
			: m_CallbackFunc(std::make_shared<std::function<RetVal(Args...)>>(std::forward<Func>(func))) {
		}

		template<typename Func, typename = typename std::enable_if<
			!std::is_same<typename std::decay<Func>::type, Callback>::value>::type>
		Callback& operator=(Func&& func)
		{
			m_CallbackFunc = std::make_shared<std::function<RetVal(Args...)>>(std::forward<Func>(func));

			return *this;
		}

		Callback(Callback&& other) noexcept
			: m_CallbackFunc(std::move(other.m_CallbackFunc))
		{
			other.m_CallbackFunc = nullptr;
		}

		Callback& operator=(Callback&& other) noexcept 
		{
			if (this != &other) 
			{
				m_CallbackFunc = std::move(other.m_CallbackFunc);
				other.m_CallbackFunc = nullptr;
			}
			return *this;
		}

		Callback(Callback& other) noexcept
			: m_CallbackFunc(other.m_CallbackFunc)
		{

		}

		Callback& operator=(Callback& other) noexcept
		{
			if (this != &other)
			{
				m_CallbackFunc = other.m_CallbackFunc;
			}
			return *this;
		}

		template<typename R = RetVal>
		typename std::enable_if<!std::is_void<R>::value, R>::type
		operator()(Args... args) const
		{
			if (m_CallbackFunc)
				return (*m_CallbackFunc)(std::forward<Args>(args)...);
			else
			{
				RLS_ASSERT(false, "[Callback]: No callback function set.")
				return {};
			}
		}

		Callback& operator=(const Callback& other) 
		{
			if (this != &other) 
			{
				if (other.m_CallbackFunc) 
					m_CallbackFunc = std::make_shared<std::function<RetVal(Args...)>>(*other.m_CallbackFunc);
				else 
					m_CallbackFunc.reset();
			}
			return *this;
		}

		template<typename R = RetVal>
		typename std::enable_if<std::is_void<R>::value, R>::type
		operator()(Args... args) const
		{
			if (m_CallbackFunc)
				(*m_CallbackFunc)(std::forward<Args>(args)...);
			else
			{
				RLS_ASSERT(false, "[Callback]: No callback function set.")
			}
		}

		[[nodiscard]] bool IsSet() const noexcept { return m_CallbackFunc != nullptr; }
		void Clear() noexcept { m_CallbackFunc = nullptr; }
		
		template<typename R = RetVal>
		typename std::enable_if<std::is_void<R>::value, void>::type
		ExecuteIfSet(Args... args) const
		{
			if (m_CallbackFunc)
				(*m_CallbackFunc)(std::forward<Args>(args)...);
		}

	private:
		std::shared_ptr<std::function<RetVal(Args...)>> m_CallbackFunc = nullptr;
	};
}