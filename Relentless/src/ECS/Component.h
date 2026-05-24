#pragma once
#include "Assets/AssetMeta.h"

#include "Callback/CoreBroadcasters.h"
#include "Core/DLLExport.h"
#include "Core/Folder.h"

#include "ECSCommon.h"

#include "Graphics/RHI/Texture.h"

#include "Math/MathTypes.h"

#include "Property/PropertyUtils.h"

#include <StaticTypeInfo/type_index.h>

#include "Utility/Common.h"
#include "Utility/StringUtils.h"

namespace Relentless
{
	class EntityManager;
	class Environment;
	class Scene;

	struct RLS_API IComponent
	{
		virtual  ~IComponent() noexcept = default;
		virtual const TypeIndex& GetStaticType() const noexcept = 0;
	};

	template<typename ComponentType>
	struct RLS_API ComponentBase : public IComponent
	{
		virtual ~ComponentBase() noexcept override = default;

		virtual const TypeIndex& GetStaticType() const noexcept override final
		{
			return StaticType();
		}

		static constexpr const TypeIndex& StaticType()
		{
			static constexpr TypeIndex typeIndex = getTypeIndex<ComponentType>();
			return typeIndex;
		}

	protected:
		void BroadcastPropertyChanged(uint64 aProperty) noexcept
		{
			CoreObjectBroadcasters::OnEntityComponentPropertyChanged(EditorSelf, ComponentType::StaticType(), this, aProperty);
		}

	private:
		friend class EntityManager;
		entity EditorSelf = NULL_ENTITY;
	};

	template<typename ComponentType>
	struct RLS_API ManagedComponent : public ComponentBase<ComponentType>
	{
		ManagedComponent(ManagedComponent&&) noexcept = default;
		ManagedComponent& operator=(ManagedComponent&&) noexcept = default;

		ManagedComponent(const ManagedComponent& aOtherComponent) = delete;
		ManagedComponent& operator=(const ManagedComponent& aOtherComponent) = delete;

		virtual void CopyFrom(const ComponentType& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) = 0;
	protected:
		ManagedComponent() = default;
		virtual ~ManagedComponent() = default;

		virtual void OnBound() noexcept {}
	protected:
		friend class EntityManager;

		entity m_Self = NULL_ENTITY;
		EntityManager* m_EntityManager = nullptr;
	};

	struct IDComponent
	{
		IDComponent()
		{
			UuId = CreateUUID();
		}
		
		IDComponent(const UUID& id)
			: UuId{id}
		{

		}

		UUID UuId = NULL_UUID;
	};

	struct CameraComponent
	{
		Matrix WorldToView			= Matrix::Identity;
		Matrix ViewToClip			= Matrix::Identity;
		float FieldOfViewDegrees	= 60.0f;
		float ClippingPlaneNear		= 0.1f;
		float ClippingPlaneFar		= 1000.0f;
		bool IsMainCamera			= false;
	};
	
	struct ParentComponent
	{
		std::vector<entity> Children;
	};

	struct IsChildComponent
	{
		explicit IsChildComponent() noexcept
			: Parent{ NULL_ENTITY }
		{} 

		entity Parent;
	};

	struct RootComponent{};
	struct SelectedInEditorComponent{};
	struct HiddenInGameComponent{};
	struct EntityDeleteRequestComponent{};

	struct FolderComponent
	{
		Folder Folder;
	};
}