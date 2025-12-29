#pragma once
#include "ISubsystem.h"
#include <StaticTypeInfo/type_index.h>

namespace Relentless
{
	using namespace static_type_info;
	using TypeIndex = static_type_info::TypeIndex;

	class ISystemManager
	{
	public:
		virtual ~ISystemManager() noexcept = default;

		template<typename SystemType>
		NO_DISCARD SystemType* GetSubsystem() noexcept
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

	private:
		std::unordered_map<TypeIndex, UniquePtr<ISubsystem>> m_Subsystems;
	};
}