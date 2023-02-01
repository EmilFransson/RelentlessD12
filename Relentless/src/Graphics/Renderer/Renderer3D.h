#pragma once
#include "../../ECS/EntityManager.h"
namespace Relentless
{
	struct VP
	{
		DirectX::XMFLOAT4X4 VPMatrix;
	};

	struct World
	{
		DirectX::XMFLOAT4X4 WorldMatrix;
	};

	struct PerFrameData
	{
		uint32_t PostProcessTextureIndex;
	};

	struct PerFrameData2
	{
		uint32_t cameraDataIndex;
		uint32_t lightMetaDataIndex;
	};

	struct LightMetaData
	{
		uint32_t directionalLightDataIndex[64];
		uint32_t pointLightDataIndex[512];
		uint32_t nrOfDirectionalLights;
		uint32_t nrOfPointLights;
	};

	struct PerDrawData2
	{
		uint32_t colorIndex;
	};

	struct PerDrawData
	{
		uint32_t vertexBufferIndex;
		uint32_t indexBufferIndex;
	};

	struct Identifier
	{
		uint32_t entityID;
	};

	class RenderTexture;
	class PerspectiveCamera;
	class Renderer3D
	{
	public:
		static void Initialize() noexcept;
		static void Begin(const std::shared_ptr<PerspectiveCamera>& pSceneCamera, EntityManager& entityManager) noexcept;
		static void Submit(const entity e) noexcept;
		static void End(const EntityManager& entityManager) noexcept;
		static void PrepareBackBuffer() noexcept;
		static void ExecuteCommands() noexcept;
		static void WaitAndSync() noexcept;
		static void WaitForGPU() noexcept;
		static void OnShutDown() noexcept;
		static [[nodiscard]] const std::shared_ptr<RenderTexture>& GetViewportTexture() noexcept;
		static void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
		static void CreateMainRootSignature() noexcept;
		static void CreatePickingRootSignature() noexcept;
		static void CreateMainPipelineState() noexcept;
		static void CreatePickingPipelineState() noexcept;
		static [[nodiscard]] entity GetHoveredEntity(const uint32_t x, const uint32_t y) noexcept;
	private:
		Renderer3D() noexcept = delete;
		~Renderer3D() noexcept = delete;
	};
}