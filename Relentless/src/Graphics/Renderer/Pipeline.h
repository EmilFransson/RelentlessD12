#pragma once
#include "Properties.h"
#include "../Shaders/Shader.h"
#include "FrameBuffer.h"
namespace Relentless
{
	struct PipelineSpecification
	{
		std::string DebugName{"?"};
		std::shared_ptr<Shader> pVertexShader{ nullptr };
		std::shared_ptr<Shader> pPixelShader{ nullptr };
		std::shared_ptr<FrameBuffer> pFrameBuffer{ nullptr };
		bool BackfaceCulling{ true };
		bool DepthWrite{ true };
		FillMode FillMode{ FillMode::Solid };
		Topology Topology{ Topology::Triangle };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature{nullptr};
	};

	class Pipeline
	{
	public:
		explicit Pipeline(const PipelineSpecification& specification) noexcept;
		~Pipeline() noexcept = default;
		[[nodiscard]] static std::shared_ptr<Pipeline> Create(const PipelineSpecification& specification) noexcept;
		[[nodiscard]] ID3D12PipelineState* GetInterface() const noexcept { return m_pPSO.Get(); }
		[[nodiscard]] const Microsoft::WRL::ComPtr<ID3D12PipelineState>& GetInterface2() const noexcept { return m_pPSO; }
		[[nodiscard]] const Microsoft::WRL::ComPtr<ID3D12RootSignature>& GetRootSig() const noexcept { return m_Specification.pRootSignature; }
		[[nodiscard]] FrameBuffer* GetFrameBuffer() const noexcept { return m_Specification.pFrameBuffer.get(); }
		[[nodiscard]] const PipelineSpecification& GetSpecification() const noexcept { return m_Specification; }
	private:
		PipelineSpecification m_Specification;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPSO;
	};
}