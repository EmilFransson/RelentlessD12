#pragma once
#include "Pipeline.h"
namespace Relentless
{
	struct RenderPassSpecification
	{
		std::string DebugName{ "?" };
		std::shared_ptr<Pipeline> RenderPipeline;
	};

	struct ShaderDetails
	{
		uint32_t InputSlot;
		uint32_t NrOfIntegers;
		void* pData{ nullptr };
		D3D12_ROOT_PARAMETER_TYPE InputType;
	};
	
	class RenderPass
	{
	public:
		explicit RenderPass(const RenderPassSpecification& renderPassDescriptor) noexcept;
		~RenderPass() noexcept = default;
		static [[nodiscard]] std::shared_ptr<RenderPass> Create(const RenderPassSpecification& renderPassSpecification) noexcept;
		[[nodiscard]] const RenderPassSpecification& GetDescriptor() const noexcept { return m_RenderPassSpecification; }
		[[nodiscard]] const std::shared_ptr<Pipeline>& GetPipeline() const noexcept { return m_RenderPassSpecification.RenderPipeline; }
		[[nodiscard]] uint32_t GetInputSlot(std::string_view inputName) noexcept;
		void Upload(std::string_view inputName, void* pData, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept;
		[[nodiscard]] std::shared_ptr<RenderTexture> GetOutput(const uint32_t index) const noexcept { return m_RenderPassSpecification.RenderPipeline->GetFrameBuffer()->GetOutput(index); }
		[[nodiscard]] std::shared_ptr<DepthStencil> GetDepthOutput() const noexcept { return m_RenderPassSpecification.RenderPipeline->GetFrameBuffer()->GetDepthOutput(); }
		[[nodiscard]] const std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC>& GetAllOutputs() const noexcept { return m_RenderTargets; }
		[[nodiscard]] const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC& GetDepthOutput2() const noexcept { return m_DepthTarget; }
		void Resize(const uint32_t width, const uint32_t height) noexcept;
		void OnMSAAReconfiguration(uint8_t samples) noexcept;
	private:
		RenderPassSpecification m_RenderPassSpecification;
		std::unordered_map<std::string, ShaderDetails> m_InputNameToInputDetails;
		std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> m_RenderTargets;
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC m_DepthTarget;
	};
}