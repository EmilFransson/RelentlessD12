#pragma once
#include "Assets/AssetMeta.h"

#include "Callback/CoreBroadcasters.h"
#include "Core/DLLExport.h"
#include "Core/StaticTypeInfo.h"

#include "Property/PropertyUtils.h"

#include "Serialization/Archive.h"

#include "Utility/Common.h"

namespace Relentless
{
	class RLS_API IAsset : public RefCounted<IAsset>
	{
	public:
		explicit IAsset(const UUID& aUUID = CreateUUID()) noexcept;
		virtual ~IAsset() noexcept = default;
		NO_DISCARD const String& GetName() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		NO_DISCARD bool IsDirty() const noexcept;

		bool Load();

		void MarkDirty() noexcept;
		
		virtual void PreLoad() {}
		virtual void PreSave() {}
		virtual void PostLoad() {}
		virtual void PostSave() {}

		bool Save();
		void SetName(const String& aName) noexcept;
		virtual bool SerializeBulk(IArchive&) noexcept { return true; }
		virtual bool SerializeCore(IArchive&) noexcept { return true; }

		virtual const UUID& GetPersistentType() const noexcept = 0;
		virtual const TypeIndex& GetStaticType() const noexcept = 0;

		Broadcaster<void(IAsset* aAsset)> OnDestroy;
		Broadcaster<void(IAsset* aAsset, uint64 aProperty)> OnPropertyChanged;
		Broadcaster<void(IAsset* aAsset)> OnSave;
		Broadcaster<void(IAsset* aAsset)> OnSaved;
	private:
		void MarkClean() noexcept;
	private:
		String m_Name;
		UUID m_UUID = NULL_UUID;
		bool m_IsDirty = false;
	};

	template<typename AssetType>
	class AssetBase : public IAsset
	{
	public:
		virtual ~AssetBase() noexcept override
		{
			OnDestroy(this);
		}

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
			return AssetType::PersistentType();
		}

		void BroadcastPropertyChanged(uint64 aProperty) noexcept
		{
			MarkDirty();
			OnPropertyChanged(this, aProperty);
			CoreObjectBroadcasters::OnAssetPropertyChanged(this, AssetType::StaticType(), aProperty);
		}
		
		static constexpr const TypeIndex& StaticType()
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<AssetType>();
			return typeIndex;
		}

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0, 0, 0, {0} };
			return uid;
		}
	};
}