#pragma once
#include "ECS/EntityManager.h"
class StructuredBuffer;
namespace Relentless
{
	enum class LightType : uint8_t { Directional = 0, Point };
	class LightManager
	{
	public:
		explicit LightManager() noexcept;
		~LightManager() noexcept = default;
		void UpdateDirectionalLight(DirectionalLightComponent& directionalLight, entity entityID) noexcept;
		void UpdatePointLight(PointLightComponent& pointLight, entity entityID) noexcept;

		void AllocateDirectionalLight(entity entityID) noexcept;
		void AllocatePointLight(entity entityID) noexcept;
		void DeallocateDirectionalLight(entity entityID) noexcept;
		void DeallocatePointLight(entity entityID) noexcept;
		[[nodiscard]] uint32_t GetLightIndex(entity entityID) const noexcept;

		[[nodiscard]] ResourceHandle GetDirectionalLightsResourceHandle() const noexcept { return m_DirectionalLightsSBHandle; }
		[[nodiscard]] ResourceHandle GetPointLightsResourceHandle() const noexcept { return m_PointLightsSBHandle; }
	private:
		ResourceHandle m_DirectionalLightsSBHandle = NULL_RESOURCE_HANDLE;
		ResourceHandle m_PointLightsSBHandle = NULL_RESOURCE_HANDLE;
		uint32_t m_NrOfDirectionalLights = 0u;
		uint32_t m_NrOfPointLights = 0u;
		std::unordered_map<entity, uint32_t> m_EntityToLightIndex;
		std::unordered_map<uint32_t, entity> m_LightToEntityIndex;
	};
}