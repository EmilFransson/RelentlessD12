#pragma once

namespace Relentless
{
	template<typename T>
	class SharedFromThis : public std::enable_shared_from_this<T>
	{
	public:
		NO_DISCARD WeakPtr<T> GetWeakPtr() noexcept
		{
			return this->weak_from_this();
		}

		NO_DISCARD WeakPtr<const T> GetWeakPtr() const noexcept
		{
			return this->weak_from_this();
		}
	};
}