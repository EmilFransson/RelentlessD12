#pragma once
#include "Core/DLLExport.h"
#include "Utility/Common.h"
#include <StaticTypeInfo/type_index.h>

#include "Serialization/Archive.h"

namespace Relentless
{
	using namespace static_type_info;
	using TypeIndex = static_type_info::TypeIndex;

	class RLS_API IAsset : public RefCounted<IAsset>
	{
	public:
		explicit IAsset(const UUID& aUUID = CreateUUID()) noexcept;
		virtual ~IAsset() noexcept = default;
		NO_DISCARD const String& GetName() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		void SetName(const String& aName) noexcept;
		virtual bool SerializeBulk(IArchive&) noexcept { return true; }
		virtual bool SerializeCore(IArchive&) noexcept { return true; }

		virtual const UUID& GetPersistentType() const noexcept = 0;
		virtual const TypeIndex& GetStaticType() const noexcept = 0;
	private:
		String m_Name;
		UUID m_UUID;
	};

	template<typename T>
	class AssetBase : public IAsset
	{
	public:
		virtual ~AssetBase() noexcept override = default;

		AssetBase(const UUID& aUUID = CreateUUID()) noexcept
			: IAsset(aUUID)
		{
		}

		virtual const TypeIndex& GetStaticType() const noexcept override final
		{
			return StaticType();
		}

		virtual const UUID& GetPersistentType() const noexcept override final
		{
			return T::PersistentType();
		}
		
		static constexpr const TypeIndex& StaticType()
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<T>();
			return typeIndex;
		}

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0, 0, 0, {0} };
			return uid;
		}
	};
}