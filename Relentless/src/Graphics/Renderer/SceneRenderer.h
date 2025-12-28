#pragma once
#include "Scene/Scene.h"
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
		bool DisplaySelectionWireframe{ true };
		bool DisplayEditorGrid{ true };
	};

	class SceneRenderer
	{
	public:
		explicit SceneRenderer(std::shared_ptr<Scene> pScene) noexcept;
		~SceneRenderer() noexcept;
		void Initialize() noexcept;
		void Begin() noexcept;
		void SetContext(const std::shared_ptr<Scene>& pScene) noexcept;
		void IssueRenderPasses() noexcept;
		void End() noexcept;
		void OnSceneViewportChanged(const uint32_t width, const uint32_t height) noexcept;
		void SetMSAASamples(const uint8_t samples) noexcept;
		const Options& GetOptions() const noexcept { return m_Options; }
		void SetContactShadowsType(const ContactShadows contactShadowsType) noexcept;
		void ToggleSelectionWireframe() noexcept;
		void ToggleEditorGrid() noexcept;
		[[nodiscard]] GFSDK_SSAO_Parameters& GetHBAOPlusParameters() noexcept { return m_HBAOPlusParameters; }
		void InitializeHBAOPlus() noexcept;
		//void OnImGuiRender(const ImVec2& viewportDimensions);
	private:
		void PreZPass() noexcept;
		void SkyboxPass() noexcept;
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
		std::shared_ptr<Scene> m_pScene = nullptr;

		struct VP
		{
			DirectX::XMFLOAT4X4 VPMatrix;
		} m_VPData;

		struct ViewProjectionMatrices
		{
			DirectX::XMFLOAT4X4 ViewMatrix;
			DirectX::XMFLOAT4X4 ProjectionMatrix;
		} m_VPMatrices;

		struct PerFrameOpaqueGeometryData
		{
			uint32_t cameraDataIndex;
			uint32_t pointLightStructuredBufferIndex;
			uint32_t directionalLightStructuredBufferIndex;
			uint32_t nrOfDirectionalLights;
			uint32_t nrOfPointLights;
			uint32_t environmentIndex;
			uint32_t brdfLutTextureIndex;
			uint32_t irradianceMapIndex;
			uint32_t radianceMapIndex;
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
			uint32_t vertexBufferIndex;
			uint32_t indexBufferIndex;
		} m_PerDrawData;

		struct Environment
		{
			DirectX::XMFLOAT3 BackgroundColor;
		} m_Environment;

		struct SkyboxPassData
		{
			uint32_t ViewProjectionIndex;
			uint32_t SkyboxTextureIndex;
		} m_SkyboxPassData;

		struct EditorGridPassVSPerFrameData
		{
			uint32_t InstanceDataSBIndex;
			uint32_t VPMatrixConstantBufferIndex;
			uint32_t BatchDataTransformHorizontalCBIndex;
			uint32_t BatchDataTransformVerticalCBIndex;
		} m_EditorGridPassVSPerFrameData;

		std::vector<entity> m_OpaqueRenderModeEntities;
		std::vector<entity> m_CutOutRenderModeEntities;
		std::vector<entity> m_TransparentRenderModeEntities;

		void* m_pMappedReadBackBufferPointer = nullptr;

		AssetHandle m_BRDFLutTextureHandle = NULL_HANDLE;
		
		entity m_HoveredEntity{NULL_ENTITY};

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
		const int EDITOR_GRID_VERTEX_COUNT = 2;
		const int EDITOR_GRID_INSTANCE_COUNT = 800;

		TransformComponent m_EditorGridTransformComponent1;
		TransformComponent m_EditorGridTransformComponent2;
		
		GFSDK_SSAO_Parameters m_HBAOPlusParameters;
		GFSDK_SSAO_Context_D3D12* m_SSAOContext = nullptr;
	};
}