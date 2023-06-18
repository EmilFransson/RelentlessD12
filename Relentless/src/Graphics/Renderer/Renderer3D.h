#pragma once
#include "../../ECS/EntityManager.h"
namespace Relentless
{
	struct VP
	{
		DirectX::XMFLOAT4X4 VPMatrix;
	};

	struct PerFrameData
	{
		uint32_t PostProcessTextureIndex;
	};

	struct PerFrameDataOpaque
	{
		uint32_t cameraDataIndex;
		uint32_t pointLightStructuredBufferIndex;
		uint32_t directionalLightStructuredBufferIndex;
		uint32_t nrOfDirectionalLights;
		uint32_t nrOfPointLights;
	};

	struct PerDrawData2
	{
		uint32_t materialIndex;
		uint32_t worldMatrixIndex;
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

	struct EditorGridInstanceDataSBIndex
	{
		uint32_t Index;
	};

	class RenderTexture;
	class PerspectiveCamera;
	class Scene;
	class EditorGrid;
	class Renderer3D
	{
	public:
		static void Initialize() noexcept;
		static void Begin(const std::shared_ptr<PerspectiveCamera>& pSceneCamera, Scene& scene) noexcept;
		static void Submit(const entity e) noexcept;
		static void End(EntityManager& entityManager) noexcept;
		static void SubmitEditorGrid(EditorGrid* pEditorGrid) noexcept;
		static void PrepareBackBuffer() noexcept;
		static void ExecuteCommands() noexcept;
		static void WaitAndSync() noexcept;
		static void WaitForGPU() noexcept;
		static void OnShutDown() noexcept;
		static void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
		static void CreateMainRootSignature() noexcept;
		static void CreatePickingRootSignature() noexcept;
		static [[nodiscard]] entity GetHoveredEntity(const uint32_t x, const uint32_t y) noexcept;
	private:
		Renderer3D() noexcept = delete;
		~Renderer3D() noexcept = delete;
	};
}