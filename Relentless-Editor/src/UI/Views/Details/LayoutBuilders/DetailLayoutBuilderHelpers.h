#pragma once
#include "Property/PropertyHandle.h"

namespace Relentless
{
	namespace DetailLayoutBuilderHelpers
	{
		NO_DISCARD inline float GetVector2ComponentValue(const Vector2& aVector2, uint8 aIndex) noexcept
		{
			switch (aIndex)
			{
			case 0u: return aVector2.x;
			case 1u: return aVector2.y;
			default:
				RLS_ASSERT(false, "[DetailLayoutBuilderHelpers::GetVector2ComponentValue]: Invalid vector component index");
				return 0.0f;
			}
		}

		NO_DISCARD inline float GetVector3ComponentValue(const Vector3& aVector3, uint8 aIndex) noexcept
		{
			switch (aIndex)
			{
			case 0u: return aVector3.x;
			case 1u: return aVector3.y;
			case 2u: return aVector3.z;
			default:
				RLS_ASSERT(false, "[DetailLayoutBuilderHelpers::GetVector3ComponentValue]: Invalid vector component index");
				return 0.0f;
			}
		}

		NO_DISCARD inline float GetVector4ComponentValue(const Vector4& aVector3, uint8 aIndex) noexcept
		{
			switch (aIndex)
			{
			case 0u: return aVector3.x;
			case 1u: return aVector3.y;
			case 2u: return aVector3.z;
			case 3u: return aVector3.w;
			default:
				RLS_ASSERT(false, "[DetailLayoutBuilderHelpers::GetVector4ComponentValue]: Invalid vector component index");
				return 0.0f;
			}
		}

		inline void SetVector2ComponentValue(Vector2& aVector2, uint8 aIndex, float aValue) noexcept
		{
			switch (aIndex)
			{
			case 0u: aVector2.x = aValue; break;
			case 1u: aVector2.y = aValue; break;
			default:
				RLS_ASSERT(false, "[DetailLayoutBuilderHelpers::SetVector2ComponentValue]: Invalid vector component index");
				break;
			}
		}

		inline void SetVector3ComponentValue(Vector3& aVector3, uint8 aIndex, float aValue) noexcept
		{
			switch (aIndex)
			{
			case 0u: aVector3.x = aValue; break;
			case 1u: aVector3.y = aValue; break;
			case 2u: aVector3.z = aValue; break;
			default:
				RLS_ASSERT(false, "[DetailLayoutBuilderHelpers::SetVector3ComponentValue]: Invalid vector component index");
				break;
			}
		}

		inline void SetVector4ComponentValue(Vector4& aVector4, uint8 aIndex, float aValue) noexcept
		{
			switch (aIndex)
			{
			case 0u: aVector4.x = aValue; break;
			case 1u: aVector4.y = aValue; break;
			case 2u: aVector4.z = aValue; break;
			case 3u: aVector4.w = aValue; break;
			default:
				RLS_ASSERT(false, "[DetailLayoutBuilderHelpers::SetVector4ComponentValue]: Invalid vector component index");
				break;
			}
		}

		NO_DISCARD inline Ref<PropertyHandle<float>> MakeVector2ComponentHandle(PropertyHandle<Vector2>* aParentHandle, uint8 aComponentIndex) noexcept
		{
			RLS_ASSERT(aComponentIndex >= 0 && aComponentIndex < 2, "[DetailLayoutBuilderHelpers::MakeVector2ComponentHandle]: Invalid vector component index");

			auto getter = [aParentHandle, aComponentIndex]() -> float
				{
					Vector2 value = Vector2::Zero;
					aParentHandle->GetValue(value);
					return GetVector2ComponentValue(value, aComponentIndex);
				};

			auto setter = [aParentHandle, aComponentIndex](const float& aValue)
				{
					Vector2 value = Vector2::Zero;
					const EPropertyAccessResult accessResult = aParentHandle->GetValue(value);
					if (accessResult == EPropertyAccessResult::Success)
					{
						SetVector2ComponentValue(value, aComponentIndex, aValue);
						aParentHandle->SetValue(value);
					}
				};

			auto defaultGetter = [aParentHandle, aComponentIndex]() -> float
				{
					Vector2 defaultValue = Vector2::Zero;
					aParentHandle->GetDefaultValue(defaultValue);
					return GetVector2ComponentValue(defaultValue, aComponentIndex);
				};

			return RLS_NEW PropertyHandle<float>(std::move(getter), std::move(setter), std::move(defaultGetter));
		}

		NO_DISCARD inline Ref<PropertyHandle<float>> MakeVector3ComponentHandle(PropertyHandle<Vector3>* aParentHandle, uint8 aComponentIndex) noexcept
		{
			RLS_ASSERT(aComponentIndex >= 0 && aComponentIndex < 3, "[DetailLayoutBuilderHelpers::MakeVector3ComponentHandle]: Invalid vector component index");

			auto getter = [aParentHandle, aComponentIndex]() -> float
				{
					Vector3 value = Vector3::Zero;
					aParentHandle->GetValue(value);
					return GetVector3ComponentValue(value, aComponentIndex);
				};

			auto setter = [aParentHandle, aComponentIndex](const float& aValue)
				{
					Vector3 value = Vector3::Zero;
					const EPropertyAccessResult accessResult = aParentHandle->GetValue(value);
					if (accessResult == EPropertyAccessResult::Success)
					{
						SetVector3ComponentValue(value, aComponentIndex, aValue);
						aParentHandle->SetValue(value);
					}
				};

			auto defaultGetter = [aParentHandle, aComponentIndex]() -> float
				{
					Vector3 defaultValue = Vector3::Zero;
					aParentHandle->GetDefaultValue(defaultValue);
					return GetVector3ComponentValue(defaultValue, aComponentIndex);
				};

			return RLS_NEW PropertyHandle<float>(std::move(getter), std::move(setter), std::move(defaultGetter));
		}

		NO_DISCARD inline Ref<PropertyHandle<float>> MakeVector4ComponentHandle(PropertyHandle<Vector4>* aParentHandle, uint8 aComponentIndex) noexcept
		{
			RLS_ASSERT(aComponentIndex >= 0 && aComponentIndex < 4, "[DetailLayoutBuilderHelpers::MakeVector4ComponentHandle]: Invalid vector component index");

			auto getter = [aParentHandle, aComponentIndex]() -> float
				{
					Vector4 value = Vector4::Zero;
					aParentHandle->GetValue(value);
					return GetVector4ComponentValue(value, aComponentIndex);
				};

			auto setter = [aParentHandle, aComponentIndex](const float& aValue)
				{
					Vector4 value = Vector4::Zero;
					const EPropertyAccessResult accessResult = aParentHandle->GetValue(value);
					if (accessResult == EPropertyAccessResult::Success)
					{
						SetVector4ComponentValue(value, aComponentIndex, aValue);
						aParentHandle->SetValue(value);
					}
				};

			auto defaultGetter = [aParentHandle, aComponentIndex]() -> float
				{
					Vector4 defaultValue = Vector4::Zero;
					aParentHandle->GetDefaultValue(defaultValue);
					return GetVector4ComponentValue(defaultValue, aComponentIndex);
				};

			return RLS_NEW PropertyHandle<float>(std::move(getter), std::move(setter), std::move(defaultGetter));
		}
	}
}