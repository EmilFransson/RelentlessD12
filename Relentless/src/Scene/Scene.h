#pragma once
#include "..\ECS\EntityManager.h"
namespace Relentless
{
	class Scene
	{
	public:
		Scene(const char* name = "Sample Scene") noexcept;
		~Scene() noexcept = default;

		entity CreateEntity(const char* tag) noexcept;
		entity CreateEntityWithUUID(const char* tag) noexcept;
		entity CreateLight(const char* name, LightComponent::Type type) noexcept;
		void DestroyEntity(const entity entityHandle) noexcept;
		[[nodiscard]] EntityManager& GetEntityManager() noexcept { return m_EntityManager; }
		[[nodiscard]] constexpr const char* GetName() const noexcept { return m_Name; }
	private:
		EntityManager m_EntityManager;
		const char* m_Name;
	};
}