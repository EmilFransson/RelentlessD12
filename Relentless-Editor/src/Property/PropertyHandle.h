#pragma once
#include <Relentless.h>
#include "IPropertyHandleBase.h"

namespace Relentless
{
	enum class EPropertyAccessResult : uint8 { Success = 0u, Fail, MixedValues };

	template<typename Type>
	class PropertyHandle : public IPropertyHandleBase
	{
	public:
		PropertyHandle(Callback<Type()> aGetter, Callback<void(const Type&)> aSetter) noexcept;
		PropertyHandle(Callback<Type()> aGetter, Callback<void(const Type&)> aSetter, Callback<Type()> aDefaultGetter) noexcept;
		PropertyHandle(Callback<Type()> aGetter, Callback<void(const Type&)> aSetter, const Type& aDefaultValue) noexcept;
		PropertyHandle() noexcept = default;

		virtual ~PropertyHandle() noexcept override = default;

		NO_DISCARD bool CanResetToDefault() const noexcept;

		NO_DISCARD virtual bool DiffersFromDefault() const noexcept;

		NO_DISCARD virtual EPropertyAccessResult GetValue(Type& aOutValue) const noexcept;

		virtual void ResetToDefault() noexcept;

		virtual void SetValue(const Type& aValue) noexcept;

		Broadcaster<void(const Type& aNewValue)> OnValueChanged;
	protected:
		Callback<Type()> m_Getter;
		Callback<void(const Type&)> m_Setter;

		std::optional<Callback<Type()>> m_DefaultGetter;
	};

	template<typename Type>
	PropertyHandle<Type>::PropertyHandle(Callback<Type()> aGetter, Callback<void(const Type&)> aSetter) noexcept
		:m_Getter(std::move(aGetter)),
		 m_Setter(std::move(aSetter))
	{
	}

	template<typename Type>
	PropertyHandle<Type>::PropertyHandle(Callback<Type()> aGetter, Callback<void(const Type&)> aSetter, Callback<Type()> aDefaultGetter) noexcept
		:m_Getter(std::move(aGetter)),
		 m_Setter(std::move(aSetter)),
		 m_DefaultGetter(std::move(aDefaultGetter))
	{
	}

	template<typename Type>
	PropertyHandle<Type>::PropertyHandle(Callback<Type()> aGetter, Callback<void(const Type&)> aSetter, const Type& aDefaultValue) noexcept
		:m_Getter(std::move(aGetter)),
		 m_Setter(std::move(aSetter))
	{
		m_DefaultGetter = [aDefaultValue]() { return aDefaultValue; };
	}

	template<typename Type>
	bool PropertyHandle<Type>::CanResetToDefault() const noexcept
	{
		return m_DefaultGetter.has_value();
	}

	template<typename Type>
	bool PropertyHandle<Type>::DiffersFromDefault() const noexcept
	{
		if (!CanResetToDefault())
			return false;

		if constexpr (std::is_arithmetic_v<Type>)
			return !Math::AreValuesClose<Type>(m_Getter(), m_DefaultGetter.value()());
		else
			return m_Getter() != (m_DefaultGetter.value())();
	}

	template<typename Type>
	EPropertyAccessResult PropertyHandle<Type>::GetValue(Type& aOutValue) const noexcept
	{
		if (!m_Getter.IsSet())
			return EPropertyAccessResult::Fail;

		aOutValue = m_Getter();

		return EPropertyAccessResult::Success;
	}

	template<typename Type>
	void PropertyHandle<Type>::ResetToDefault() noexcept
	{
		if (!CanResetToDefault())
			return;

		SetValue((m_DefaultGetter.value())());
	}

	template<typename Type>
	void PropertyHandle<Type>::SetValue(const Type& aValue) noexcept
	{
		m_Setter(aValue);
		OnValueChanged(aValue);
	}
}