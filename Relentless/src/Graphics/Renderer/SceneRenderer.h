#pragma once
#include "RenderPass.h"
#include "../../Scene/Scene.h"
#include "../Resources/Buffer.h"
namespace Relentless
{
	class SceneRenderer
	{
	public:
		explicit SceneRenderer(std::shared_ptr<Scene> pScene) noexcept;
		~SceneRenderer() noexcept = default;
		void Initialize() noexcept;
		void Begin() noexcept;
		void IssueRenderPasses() noexcept;
		void End() noexcept;
		void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
		entity GetHoveredEntity() noexcept;
	private:
		void GeometryPass() noexcept;
		void EditorGridPass() noexcept;
		void WireframePass() noexcept;
		void PickingPass() noexcept;
		void CompositePass() noexcept;
	private:
		std::shared_ptr<Scene> m_pScene;

		struct VP
		{
			DirectX::XMFLOAT4X4 VPMatrix;
		} m_VPData;

		struct PerFrameOpaqueGeometryData
		{
			uint32_t cameraDataIndex;
			uint32_t pointLightStructuredBufferIndex;
			uint32_t directionalLightStructuredBufferIndex;
			uint32_t nrOfDirectionalLights;
			uint32_t nrOfPointLights;
		} m_PerFrameOpaqueGeometryData;

		struct PerFrameEditorData
		{
			uint32_t cameraDataIndex;
		} m_PerFrameEditorData;

		struct CompositeData
		{
			uint32_t PostProcessTextureIndex;
		} m_CompositeData;

		struct PickingData
		{
			uint32_t entityID;
		} m_PickingData;

		struct PerDrawData
		{
			uint32_t materialIndex;
			uint32_t worldMatrixIndex;
		} m_PerDrawData;

		struct EditorBatchData
		{
			uint32_t worldMatrixIndex1;
			uint32_t worldMatrixIndex2;
		} m_EditorBatchData;

		struct InstanceDataSBIndex
		{
			uint32_t Index;
		} m_InstanceDataSBIndex;

		std::shared_ptr<RenderPass> m_GeometryRenderPass;
		std::shared_ptr<RenderPass> m_GeometryPickingRenderPass;
		std::shared_ptr<RenderPass> m_CompositeRenderPass;
		std::shared_ptr<RenderPass> m_WireFrameRenderPass;
		std::shared_ptr<RenderPass> m_EditorGridRenderPass;

		std::shared_ptr<ReadBackBuffer> m_pIdentifierReadbackBuffer{ nullptr };
		std::shared_ptr<RenderTexture> m_pResolvedTexture{ nullptr };

		entity m_HoveredEntity{NULL_ENTITY};

		//EditorGrid
		struct Vertex_Basic
		{
			DirectX::XMFLOAT3 Position;
		};

		struct InstanceData
		{
			DirectX::XMFLOAT3 Position;
			float padding1;
			struct
			{
				float R;
				float G;
				float B;
			} Color;
			float padding2;
		};

		#define EDITOR_GRID_VERTEX_COUNT 2
		#define EDITOR_GRID_INSTANCE_COUNT 800

		std::unique_ptr<VertexBuffer> m_pEditorGridVertexBuffer{nullptr};
		std::unique_ptr<StructuredBuffer> m_pEditorGridInstanceDataStructuredBuffer{nullptr};
		std::array<Vertex_Basic, EDITOR_GRID_VERTEX_COUNT> m_EditorGridVertices;

		TransformComponent m_EditorGridTransformComponent1;
		TransformComponent m_EditorGridTransformComponent2;
	};
}