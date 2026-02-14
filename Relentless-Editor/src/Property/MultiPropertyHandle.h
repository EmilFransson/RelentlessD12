#pragma once
#include "PropertyHandle.h"

namespace Relentless
{
	template<typename DataType>
	class MultiPropertyHandle : public PropertyHandle<DataType>
	{
	public:
		MultiPropertyHandle(Callback<DataType(uint32)> aGetter, Callback<void(const DataType&, uint32)> aSetter, uint32 aItemCount) noexcept;
		MultiPropertyHandle(Callback<DataType(uint32)> aGetter, Callback<void(const DataType&, uint32)> aSetter, uint32 aItemCount, Callback<DataType()> aDefaultGetter) noexcept;
		MultiPropertyHandle(Callback<DataType(uint32)> aGetter, Callback<void(const DataType&, uint32)> aSetter, uint32 aItemCount, const DataType& aDefaultValue) noexcept;

		virtual ~MultiPropertyHandle() noexcept override = default;

		NO_DISCARD virtual bool AllEqualTo(const DataType& aValue) const noexcept;

		NO_DISCARD virtual bool DiffersFromDefault() const noexcept override;

		NO_DISCARD virtual uint32 GetItemCount() const noexcept;
		NO_DISCARD virtual EPropertyAccessResult GetValue(DataType& aOutValue) const noexcept override;

		virtual void ResetToDefault() noexcept override;

		virtual void SetValue(const DataType& aValue) noexcept override;
	protected:
		Callback<DataType(uint32)> m_MultiGetter;
		Callback<void(const DataType&, uint32)> m_MultiSetter;
		uint32 m_ItemCount = 0u;
	};

	template<typename DataType>
	MultiPropertyHandle<DataType>::MultiPropertyHandle(Callback<DataType(uint32)> aGetter, Callback<void(const DataType&, uint32)> aSetter, uint32 aItemCount) noexcept
		:m_MultiGetter(std::move(aGetter)),
		 m_MultiSetter(std::move(aSetter)),
		 m_ItemCount(aItemCount)
	{
	}

	template<typename DataType>
	MultiPropertyHandle<DataType>::MultiPropertyHandle(Callback<DataType(uint32)> aGetter, Callback<void(const DataType&, uint32)> aSetter, uint32 aItemCount, Callback<DataType()> aDefaultGetter) noexcept
		:m_MultiGetter(std::move(aGetter)),
		 m_MultiSetter(std::move(aSetter)),
		 m_ItemCount(aItemCount)
	{
		this->m_DefaultGetter = std::move(aDefaultGetter);
	}

	template<typename DataType>
	MultiPropertyHandle<DataType>::MultiPropertyHandle(Callback<DataType(uint32)> aGetter, Callback<void(const DataType&, uint32)> aSetter, uint32 aItemCount, const DataType& aDefaultValue) noexcept
		:m_MultiGetter(std::move(aGetter)),
		 m_MultiSetter(std::move(aSetter)),
		 m_ItemCount(aItemCount)
	{
		this->m_DefaultGetter = [aDefaultValue]() { return aDefaultValue; };
	}

	template<typename DataType>
	bool MultiPropertyHandle<DataType>::AllEqualTo(const DataType& aValue) const noexcept
	{
		for (uint32 i = 0u; i < m_ItemCount; ++i)
		{
			if constexpr (std::is_arithmetic_v<DataType>)
			{
				if (!Math::AreValuesClose(m_MultiGetter(i), aValue))
					return false;
			}
			else
			{
				if (m_MultiGetter(i) != aValue)
					return false;
			}
		}

		return true;
	}

	template<typename DataType>
	bool MultiPropertyHandle<DataType>::DiffersFromDefault() const noexcept
	{
		if (!this->CanResetToDefault())
			return false;

		if (!AllEqualTo(this->m_DefaultGetter.value()()))
			return true;

		return false;
	}

	template<typename DataType>
	uint32 MultiPropertyHandle<DataType>::GetItemCount() const noexcept
	{
		return m_ItemCount;
	}

	template<typename DataType>
	EPropertyAccessResult MultiPropertyHandle<DataType>::GetValue(DataType& aOutValue) const noexcept
	{
		if (GetItemCount() == 0u)
			return EPropertyAccessResult::Fail;

		if (!m_MultiGetter.IsSet())
			return EPropertyAccessResult::Fail;

		const DataType value = m_MultiGetter(0);
		if (!AllEqualTo(value))
			return EPropertyAccessResult::MixedValues;

		aOutValue = value;
		return EPropertyAccessResult::Success;
	}

	template<typename DataType>
	void MultiPropertyHandle<DataType>::ResetToDefault() noexcept
	{
		if (!this->CanResetToDefault())
			return;

		SetValue(this->m_DefaultGetter.value()());
	}

	template<typename DataType>
	void MultiPropertyHandle<DataType>::SetValue(const DataType& aValue) noexcept
	{
		for (uint32 i = 0u; i < m_ItemCount; ++i)
			m_MultiSetter(aValue, i);

		this->OnValueChanged(aValue);
	}
}