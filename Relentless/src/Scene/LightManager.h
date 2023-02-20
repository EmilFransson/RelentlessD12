#pragma once
#include "..\ECS\EntityManager.h"
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

		[[nodiscard]] const std::unique_ptr<StructuredBuffer>& GetDirectionalLights() const noexcept { return m_DirectionalLights; }
		[[nodiscard]] const std::unique_ptr<StructuredBuffer>& GetPointLights() const noexcept { return m_PointLights; }
	private:
		std::unique_ptr<StructuredBuffer> m_DirectionalLights;
		std::unique_ptr<StructuredBuffer> m_PointLights;
		std::unordered_map<entity, uint32_t> m_EntityToLightIndex;
		std::unordered_map<uint32_t, entity> m_LightToEntityIndex;
	};
}