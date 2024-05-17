#include "RenderPass.h"
#include "../D3D12Core.h"
#include "Properties.h"
namespace Relentless
{
	[[nodiscard]] static D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE RLSOperatorToD3D12Operator(OperatorOnLoad op) noexcept
	{
		switch (op)
		{
		case OperatorOnLoad::Clear: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		case OperatorOnLoad::LoadOnly: return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		}

		RLS_ASSERT(false, "Unknown operator encountered.");
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
	}

	RenderPass::RenderPass(const RenderPassSpecification& renderPassSpecification) noexcept
		: m_RenderPassSpecification{ renderPassSpecification }
	{
		RLS_ASSERT(renderPassSpecification.RenderPipeline, "No valid pipeline submitted for render pass.");
	
		auto& reflectionResult = m_RenderPassSpecification.RenderPipeline->GetShaderReflectionResults();
		for (auto& [rootSigSlot, reflectionData] : reflectionResult.RootSignatureSlotToReflectionDataMap)
		{
			m_InputNameToInputDetails[reflectionData.Name] = {
				.InputSlot = rootSigSlot,
				.NrOfIntegers = reflectionData.NrOfIntegers,
				.InputType = HLSLParameterTypeToD3D12RootParameterType(reflectionData.shaderInputType)
			};
		}

		auto fbSpec = m_RenderPassSpecification.RenderPipeline->GetFrameBuffer()->GetSpecification();
		for (auto& outputAttachment : fbSpec.Attachments.ColorAttachments)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess{};
			beginningAccess.Clear.ClearValue.Color[0] = outputAttachment.ClearColor.x;
			beginningAccess.Clear.ClearValue.Color[1] = outputAttachment.ClearColor.y;
			beginningAccess.Clear.ClearValue.Color[2] = outputAttachment.ClearColor.z;
			beginningAccess.Clear.ClearValue.Color[3] = outputAttachment.ClearColor.w;
			beginningAccess.Clear.ClearValue.Format = RLSTextureFormatToDXGITextureFormat(outputAttachment.Format);
			beginningAccess.Type = RLSOperatorToD3D12Operator(outputAttachment.OperatorOnLoad);

			D3D12_RENDER_PASS_ENDING_ACCESS endingAccess{};
			endingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc{};
			rtDesc.cpuDescriptor = outputAttachment.Output->GetRTVDescriptorHandle().CPUHandle;
			rtDesc.BeginningAccess = beginningAccess;
			rtDesc.EndingAccess = endingAccess;

			m_RenderTargets.push_back(rtDesc);
		}

		if (fbSpec.Attachments.DepthAttachment.Output)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess{};
			beginningAccess.Clear.ClearValue.DepthStencil.Depth = 1.0f;
			beginningAccess.Clear.ClearValue.DepthStencil.Stencil = 0.0f;
			beginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			beginningAccess.Type = RLSOperatorToD3D12Operator(fbSpec.Attachments.DepthAttachment.OperatorOnLoad);

			m_DepthTarget.cpuDescriptor = fbSpec.Attachments.DepthAttachment.Output->GetDSVDescriptorHandle().CPUHandle;
			m_DepthTarget.DepthBeginningAccess = beginningAccess;
			m_DepthTarget.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		
			D3D12_RENDER_PASS_BEGINNING_ACCESS stencilBeginningAccess{};
			stencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;

			D3D12_RENDER_PASS_ENDING_ACCESS stencilEndingAccess{};
			stencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

			m_DepthTarget.StencilBeginningAccess = stencilBeginningAccess;
			m_DepthTarget.StencilEndingAccess = stencilEndingAccess;
		}
	}

	std::shared_ptr<RenderPass> RenderPass::Create(const RenderPassSpecification& renderPassSpecification) noexcept
	{
		return std::make_shared<RenderPass>(renderPassSpecification);
	}

	[[nodiscard]] uint32_t RenderPass::GetInputSlot(std::string_view inputName) noexcept
	{
		RLS_ASSERT(m_InputNameToInputDetails.find(std::string(inputName)) != m_InputNameToInputDetails.end(), "Shader input name is invalid.");

		return m_InputNameToInputDetails[std::string(inputName)].InputSlot;
	}

	void RenderPass::Upload(std::string_view inputName, void* pData, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		RLS_ASSERT(pData, "Data submitted is not valid.");
		RLS_ASSERT(m_InputNameToInputDetails.find(std::string(inputName)) != m_InputNameToInputDetails.end(), "Shader input name is invalid.");

		ShaderDetails& shaderDetails = m_InputNameToInputDetails[std::string(inputName)];
		switch (shaderDetails.InputType)
		{
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			DXCall_STD(pCommandList->SetGraphicsRoot32BitConstants(shaderDetails.InputSlot, shaderDetails.NrOfIntegers, pData, 0u));
			break;
		}
	}

	void RenderPass::Resize(const uint32_t width, const uint32_t height) noexcept
	{
		m_RenderPassSpecification.RenderPipeline->GetFrameBuffer()->Resize(width, height);

		m_RenderTargets.clear();

		auto fbSpec = m_RenderPassSpecification.RenderPipeline->GetFrameBuffer()->GetSpecification();
		for (auto& outputAttachment : fbSpec.Attachments.ColorAttachments)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess{};
			beginningAccess.Clear.ClearValue.Color[0] = outputAttachment.ClearColor.x;
			beginningAccess.Clear.ClearValue.Color[1] = outputAttachment.ClearColor.y;
			beginningAccess.Clear.ClearValue.Color[2] = outputAttachment.ClearColor.z;
			beginningAccess.Clear.ClearValue.Color[3] = outputAttachment.ClearColor.w;
			beginningAccess.Clear.ClearValue.Format = RLSTextureFormatToDXGITextureFormat(outputAttachment.Format);
			beginningAccess.Type = RLSOperatorToD3D12Operator(outputAttachment.OperatorOnLoad);

			D3D12_RENDER_PASS_ENDING_ACCESS endingAccess{};
			endingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc{};
			rtDesc.cpuDescriptor = outputAttachment.Output->GetRTVDescriptorHandle().CPUHandle;
			rtDesc.BeginningAccess = beginningAccess;
			rtDesc.EndingAccess = endingAccess;

			m_RenderTargets.push_back(rtDesc);
		}

		if (fbSpec.Attachments.DepthAttachment.Output)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess{};
			beginningAccess.Clear.ClearValue.DepthStencil.Depth = 1.0f;
			beginningAccess.Clear.ClearValue.DepthStencil.Stencil = 0.0f;
			beginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			beginningAccess.Type = RLSOperatorToD3D12Operator(fbSpec.Attachments.DepthAttachment.OperatorOnLoad);

			m_DepthTarget.cpuDescriptor = fbSpec.Attachments.DepthAttachment.Output->GetDSVDescriptorHandle().CPUHandle;
			m_DepthTarget.DepthBeginningAccess = beginningAccess;
			m_DepthTarget.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			D3D12_RENDER_PASS_BEGINNING_ACCESS stencilBeginningAccess{};
			stencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;

			D3D12_RENDER_PASS_ENDING_ACCESS stencilEndingAccess{};
			stencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

			m_DepthTarget.StencilBeginningAccess = stencilBeginningAccess;
			m_DepthTarget.StencilEndingAccess = stencilEndingAccess;
		}
	}

	void RenderPass::OnMSAAReconfiguration(uint8_t samples) noexcept
	{
		m_RenderPassSpecification.RenderPipeline->OnMSAAReconfiguration(samples);

		m_RenderTargets.clear();

		auto fbSpec = m_RenderPassSpecification.RenderPipeline->GetFrameBuffer()->GetSpecification();
		for (auto& outputAttachment : fbSpec.Attachments.ColorAttachments)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess{};
			beginningAccess.Clear.ClearValue.Color[0] = outputAttachment.ClearColor.x;
			beginningAccess.Clear.ClearValue.Color[1] = outputAttachment.ClearColor.y;
			beginningAccess.Clear.ClearValue.Color[2] = outputAttachment.ClearColor.z;
			beginningAccess.Clear.ClearValue.Color[3] = outputAttachment.ClearColor.w;
			beginningAccess.Clear.ClearValue.Format = RLSTextureFormatToDXGITextureFormat(outputAttachment.Format);
			beginningAccess.Type = RLSOperatorToD3D12Operator(outputAttachment.OperatorOnLoad);

			D3D12_RENDER_PASS_ENDING_ACCESS endingAccess{};
			endingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDesc{};
			rtDesc.cpuDescriptor = outputAttachment.Output->GetRTVDescriptorHandle().CPUHandle;
			rtDesc.BeginningAccess = beginningAccess;
			rtDesc.EndingAccess = endingAccess;

			m_RenderTargets.push_back(rtDesc);
		}

		if (fbSpec.Attachments.DepthAttachment.Output)
		{
			D3D12_RENDER_PASS_BEGINNING_ACCESS beginningAccess{};
			beginningAccess.Clear.ClearValue.DepthStencil.Depth = 1.0f;
			beginningAccess.Clear.ClearValue.DepthStencil.Stencil = 0.0f;
			beginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			beginningAccess.Type = RLSOperatorToD3D12Operator(fbSpec.Attachments.DepthAttachment.OperatorOnLoad);

			m_DepthTarget.cpuDescriptor = fbSpec.Attachments.DepthAttachment.Output->GetDSVDescriptorHandle().CPUHandle;
			m_DepthTarget.DepthBeginningAccess = beginningAccess;
			m_DepthTarget.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE::D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

			D3D12_RENDER_PASS_BEGINNING_ACCESS stencilBeginningAccess{};
			stencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;

			D3D12_RENDER_PASS_ENDING_ACCESS stencilEndingAccess{};
			stencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

			m_DepthTarget.StencilBeginningAccess = stencilBeginningAccess;
			m_DepthTarget.StencilEndingAccess = stencilEndingAccess;
		}

	}
}