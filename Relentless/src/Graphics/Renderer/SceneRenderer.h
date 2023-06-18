#pragma once
#include "RenderPass.h"
#include "../../Scene/Scene.h"
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
		entity GetHoveredEntity(const uint32_t x, const uint32_t y) noexcept;
	private:
		void CreateRootSignatures() noexcept; //Should be moved!
		void PreRender() noexcept;
		void GeometryPass() noexcept;
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

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pGeometryRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pGeometryPickingRootSignature{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pCompositeRootSignature{ nullptr };

		std::shared_ptr<RenderPass> m_GeometryRenderPass;
		std::shared_ptr<RenderPass> m_GeometryPickingRenderPass;
		std::shared_ptr<RenderPass> m_CompositeRenderPass;

		std::shared_ptr<ReadbackTexture> m_pIdentifierReadbackTexture{ nullptr };

		entity m_HoveredEntity{NULL_ENTITY};
	};
}