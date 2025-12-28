#pragma once
#include "Utility/Common.h"
#include <StaticTypeInfo/type_index.h>

namespace Relentless
{
	using namespace static_type_info;
	using TypeIndex = static_type_info::TypeIndex;

	class IAsset : public RefCounted<IAsset>
	{
	public:
		explicit IAsset(const UUID& aUUID = CreateUUID()) noexcept;
		virtual ~IAsset() = default;
		NO_DISCARD const String& GetName() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		void SetName(const String& aName) noexcept;

		virtual const TypeIndex& GetStaticType() const noexcept = 0;
	private:
		String m_Name;
		UUID m_UUID;
	};

	template<typename T>
	class AssetBase : public IAsset
	{
	public:
		virtual const TypeIndex& GetStaticType() const noexcept override final
		{
			return StaticType();
		}

		static constexpr const TypeIndex& StaticType()
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<T>();
			return typeIndex;
		}
	};
}