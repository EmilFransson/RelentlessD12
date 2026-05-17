#pragma once
#include "ISubsystem.h"
#include "Core/StaticTypeInfo.h"

namespace Relentless
{
	class ISystemManager
	{
	public:
		virtual ~ISystemManager() noexcept
		{
			for (const auto&[type, pSystem] : m_Subsystems)
				pSystem->OnUnload(this);
		}

		void DestroyAllSubSystems() noexcept
		{
			for (const auto& [type, pSystem] : m_Subsystems)
				pSystem->OnUnload(this);

			m_Subsystems.clear();
		}

		template<typename SystemType>
		SystemType* GetSubsystem() noexcept
		{
			static_assert(std::is_base_of_v<ISubsystem, SystemType>, "SystemType must derive from ISubsystem to be used as a subsystem.");

			static constexpr TypeIndex id = getTypeIndex<SystemType>();

			auto it = m_Subsystems.find(id);
			if (it != m_Subsystems.end())
				return static_cast<SystemType*>(it->second.get());

			if (!SystemType::ShouldCreateSubsystem(this))
				return nullptr;

			UniquePtr<SystemType> pNewSystem = MakeUnique<SystemType>();
			if (!pNewSystem->OnLoad(this))
				return nullptr;

			SystemType* ptr = pNewSystem.get();
			m_Subsystems[id] = std::move(pNewSystem);
			return ptr;
		}

		template<typename SystemType>
		const SystemType* GetSubsystem() const noexcept
		{
			static constexpr TypeIndex id = getTypeIndex<SystemType>();
			auto it = m_Subsystems.find(id);
			if (it != m_Subsystems.end())
				return static_cast<const SystemType*>(it->second.get());

			return nullptr;
		}

	private:
		std::unordered_map<TypeIndex, UniquePtr<ISubsystem>> m_Subsystems;
	};
}