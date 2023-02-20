#include "LightManager.h"
#include "..\Graphics\Resources\StructuredBuffer.h"
#include "..\Graphics\MemoryManager.h"

namespace Relentless
{
	LightManager::LightManager() noexcept
		: m_DirectionalLights{ nullptr },
		  m_PointLights{ nullptr }
	{
		m_DirectionalLights = std::make_unique<StructuredBuffer>(100u, sizeof(DirectionalLightComponent));
		m_PointLights = std::make_unique<StructuredBuffer>(100u, sizeof(PointLightComponent));
	}

	void LightManager::UpdateDirectionalLight(DirectionalLightComponent& directionalLight, entity entityID) noexcept
	{
		MemoryManager::Get().UpdateStructuredBuffer(*m_DirectionalLights, &directionalLight, GetLightIndex(entityID));
	}

	void LightManager::UpdatePointLight(PointLightComponent& pointLight, entity entityID) noexcept
	{
		MemoryManager::Get().UpdateStructuredBuffer(*m_PointLights, &pointLight, GetLightIndex(entityID));
	}

	void LightManager::AllocateDirectionalLight(entity entityID) noexcept
	{
		RLS_ASSERT(!m_EntityToLightIndex.contains(entityID), "Entity already have a light associated with it.");
		m_EntityToLightIndex[entityID] = m_DirectionalLights->GetFreeIndex();
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;
	}

	void LightManager::AllocatePointLight(entity entityID) noexcept
	{
		RLS_ASSERT(!m_EntityToLightIndex.contains(entityID), "Entity already have a light associated with it.");
		m_EntityToLightIndex[entityID] = m_PointLights->GetFreeIndex();
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;
	}

	void LightManager::DeallocateDirectionalLight(entity lightToDeallocate) noexcept
	{
		RLS_ASSERT(m_EntityToLightIndex.contains(lightToDeallocate), "Entity does not have a light associated with it.");
		uint32_t maxDirectionalLightIndex = (m_DirectionalLights->m_NrOfElements--) - 1;
		uint32_t lightIndex = GetLightIndex(lightToDeallocate);

		entity entityID = m_LightToEntityIndex[maxDirectionalLightIndex];
		m_EntityToLightIndex[entityID] = lightIndex;
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;

		m_EntityToLightIndex.erase(lightToDeallocate);
	}

	void LightManager::DeallocatePointLight(entity lightToDeallocate) noexcept
	{
		RLS_ASSERT(m_EntityToLightIndex.contains(lightToDeallocate), "Entity does not have a light associated with it.");
		uint32_t maxPointLightIndex = (m_PointLights->m_NrOfElements--) - 1;
		uint32_t lightIndex = GetLightIndex(lightToDeallocate);

		entity entityID = m_LightToEntityIndex[maxPointLightIndex];
		m_EntityToLightIndex[entityID] = lightIndex;
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;

		m_EntityToLightIndex.erase(lightToDeallocate);
	}

	uint32_t LightManager::GetLightIndex(entity entityID) const noexcept
	{
		RLS_ASSERT(m_EntityToLightIndex.contains(entityID), "Entity does not have a light associated with it.");
		return m_EntityToLightIndex.at(entityID);
	}
}