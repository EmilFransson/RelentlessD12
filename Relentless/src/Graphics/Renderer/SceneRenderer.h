#pragma once
#include "RenderPass.h"
#include "../../Scene/Scene.h"
#include "../Resources/Buffer.h"
#include "../../../vendor/includes/HBAO+/GFSDK_SSAO.h"
namespace Relentless
{

	enum class ContactShadows : uint8_t { OFF = 0, HBAO_PLUS };
	struct Options
	{
		uint8_t MSAASamples{ 2u };
		#if defined RLS_DEBUG
		ContactShadows ContactShadowType{ ContactShadows::OFF };
		#else
		ContactShadows ContactShadowType{ ContactShadows::HBAO_PLUS };
		#endif
	};

	class SceneRenderer
	{
	public:
		explicit SceneRenderer(std::shared_ptr<Scene> pScene) noexcept;
		~SceneRenderer() noexcept = default;
		void Initialize() noexcept;
		void Begin() noexcept;
		void SetContext(const std::shared_ptr<Scene>& pScene) noexcept;
		void IssueRenderPasses() noexcept;
		void End() noexcept;
		void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
		void SetMSAASamples(const uint8_t samples) noexcept;
		entity GetHoveredEntity() noexcept;
		const Options& GetOptions() const noexcept { return m_Options; }
		void SetContactShadowsType(const ContactShadows contactShadowsType) noexcept;
		[[nodiscard]] GFSDK_SSAO_Parameters& GetHBAOPlusParameters() noexcept { return m_HBAOPlusParameters; }
		void InitializeHBAOPlus() noexcept;
	private:
		void PreZPass() noexcept;
		void OpaqueGeometryPass() noexcept;
		void CutOutGeometryPass() noexcept;
		void TransparentGeometryPass() noexcept;
		void HBAOPlusRenderPass() noexcept;
		void EditorGridPass() noexcept;
		void WireframePass() noexcept;
		void PickingPass() noexcept;
		void CompositePass() noexcept;
		void CombinedGeometryAndPickingPass() noexcept;
	private:
		Options m_Options;
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
			uint32_t environmentIndex;
			uint32_t brdfLutTextureIndex;
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

		struct Environment
		{
			DirectX::XMFLOAT3 BackgroundColor;
		} m_Environment;

		size_t m_EnvironmentCBHandle;

		std::vector<entity> m_OpaqueRenderModeEntities;
		std::vector<entity> m_CutOutRenderModeEntities;
		std::vector<entity> m_TransparentRenderModeEntities;

		std::shared_ptr<RenderPass> m_OpaqueGeometryRenderPass;
		std::shared_ptr<RenderPass> m_CutOutGeometryRenderPass;
		std::shared_ptr<RenderPass> m_TransparentGeometryRenderPass;
		std::shared_ptr<RenderPass> m_GeometryPickingRenderPass;
		std::shared_ptr<RenderPass> m_CompositeRenderPass;
		std::shared_ptr<RenderPass> m_WireFrameRenderPass;
		std::shared_ptr<RenderPass> m_EditorGridRenderPass;
		std::shared_ptr<RenderPass> m_CombinedGeometryAndPickingPass;
		std::shared_ptr<RenderPass> m_PreZRenderPass;

		std::shared_ptr<ReadBackBuffer> m_pIdentifierReadbackBuffer{ nullptr };
		std::shared_ptr<RenderTexture> m_pResolvedTexture{ nullptr };
		TextureHandle m_BRDFLutTextureHandle;
		
		GFSDK_SSAO_Parameters m_HBAOPlusParameters;

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
		
		GFSDK_SSAO_Context_D3D12* m_SSAOContext;
	};
}