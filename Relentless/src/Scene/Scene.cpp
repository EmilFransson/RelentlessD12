#include "Scene.h"
namespace Relentless
{
	Scene::Scene(const char* name) noexcept
		: m_Name{ name }
	{

	}

	entity Scene::CreateEntity(const char* name) noexcept
	{
		return CreateEntityWithUUID(name);
	}

	entity Scene::CreateEntityWithUUID(const char* name) noexcept
	{
		auto entity = m_EntityManager.CreateEntity();
		m_EntityManager.Add<TransformComponent>(entity);
		m_EntityManager.Add<NameComponent>(entity, name);
		m_EntityManager.Add<IDComponent>(entity);

		return entity;
	}

	entity Scene::CreateLight(const char* name, LightComponent::Type type) noexcept
	{
		auto lightEntity = CreateEntityWithUUID(name);
		m_EntityManager.Add<LightComponent>(lightEntity, type);
		return lightEntity;
	}

	void Scene::DestroyEntity(const entity entityHandle) noexcept
	{
		m_EntityManager.DestroyEntity(entityHandle);
	}
}