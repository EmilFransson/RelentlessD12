#include "Graphics/MemoryManager.h"
#include "LightManager.h"
#include "Core/Application.h"

namespace Relentless
{
	LightManager::LightManager() noexcept
	{
		//ResourceManager& resourceManager = Application::Get().GetResourceManager();
		//m_DirectionalLightsSBHandle = resourceManager.CreateStructuredBufferSet("DirectionalLightsStructuredBuffer", 100u, sizeof(DirectionalLightComponent));
		//m_PointLightsSBHandle = resourceManager.CreateStructuredBufferSet("PointLightsStructuredBuffer", 100u, sizeof(PointLightComponent));
	}

	void LightManager::UpdateDirectionalLight(DirectionalLightComponent& directionalLight, entity entityID) noexcept
	{
		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();
		//Application::Get().GetResourceManager().UploadStructuredBufferData(m_DirectionalLightsSBHandle, &directionalLight, sizeof(DirectionalLightComponent), frameIndex, GetLightIndex(entityID));
	}

	void LightManager::UpdatePointLight(PointLightComponent& pointLight, entity entityID) noexcept
	{
		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();
		//Application::Get().GetResourceManager().UploadStructuredBufferData(m_PointLightsSBHandle, &pointLight, sizeof(PointLightComponent), frameIndex, GetLightIndex(entityID));
	}

	void LightManager::AllocateDirectionalLight(entity entityID) noexcept
	{
		RLS_ASSERT(!m_EntityToLightIndex.contains(entityID), "Entity already have a light associated with it.");
		m_EntityToLightIndex[entityID] = m_NrOfDirectionalLights++;
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;
	}

	void LightManager::AllocatePointLight(entity entityID) noexcept
	{
		RLS_ASSERT(!m_EntityToLightIndex.contains(entityID), "Entity already have a light associated with it.");
		m_EntityToLightIndex[entityID] = m_NrOfPointLights++;
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;
	}

	void LightManager::DeallocateDirectionalLight(entity lightToDeallocate) noexcept
	{
		RLS_ASSERT(m_EntityToLightIndex.contains(lightToDeallocate), "Entity does not have a light associated with it.");
		uint32_t maxDirectionalLightIndex = (m_NrOfDirectionalLights--) - 1;
		uint32_t lightIndex = GetLightIndex(lightToDeallocate);

		entity entityID = m_LightToEntityIndex[maxDirectionalLightIndex];
		m_EntityToLightIndex[entityID] = lightIndex;
		m_LightToEntityIndex[m_EntityToLightIndex[entityID]] = entityID;

		m_EntityToLightIndex.erase(lightToDeallocate);
	}

	void LightManager::DeallocatePointLight(entity lightToDeallocate) noexcept
	{
		RLS_ASSERT(m_EntityToLightIndex.contains(lightToDeallocate), "Entity does not have a light associated with it.");
		uint32_t maxPointLightIndex = (m_NrOfPointLights--) - 1;
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