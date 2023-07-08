#pragma once
#include "Properties.h"
#include "../Shaders/Shader.h"
#include "FrameBuffer.h"
#include "../Shaders/ShaderReflector.h"
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
		bool MSAAEligible{ false };
		FillMode FillMode{ FillMode::Solid };
		Topology Topology{ Topology::Triangle };
	};

	class Pipeline
	{
	public:
		explicit Pipeline(const PipelineSpecification& specification) noexcept;
		~Pipeline() noexcept = default;
		[[nodiscard]] static std::shared_ptr<Pipeline> Create(const PipelineSpecification& specification) noexcept;
		[[nodiscard]] ID3D12PipelineState* GetInterface() const noexcept { return m_pPSO.Get(); }
		[[nodiscard]] const Microsoft::WRL::ComPtr<ID3D12PipelineState>& GetInterface2() const noexcept { return m_pPSO; }
		[[nodiscard]] const Microsoft::WRL::ComPtr<ID3D12RootSignature>& GetRootSig() const noexcept { return m_ReflectionResult.pRootSignature; }
		[[nodiscard]] std::shared_ptr<FrameBuffer> GetFrameBuffer() const noexcept { return m_Specification.pFrameBuffer; }
		[[nodiscard]] const PipelineSpecification& GetSpecification() const noexcept { return m_Specification; }
		[[nodiscard]] const ReflectionResult& GetShaderReflectionResults() const noexcept { return m_ReflectionResult; }
		void OnMSAAReconfiguration(uint8_t nrOfSamples) noexcept;
	private:
		void Initialize(const PipelineSpecification& specification) noexcept;
	private:
		PipelineSpecification m_Specification;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPSO;
		ReflectionResult m_ReflectionResult;
	};
}