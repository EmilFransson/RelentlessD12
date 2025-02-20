#include "Pipeline.h"
#include "..\D3D12Core.h"
#include "Graphics/D3D12Debug.h"

namespace Relentless
{
	[[nodiscard]] static D3D12_COMPARISON_FUNC RLSComparisonFuncToD3D12ComparisonFunc(DepthComparisonFunction depthComparisonFunction) noexcept
	{
		switch (depthComparisonFunction)
		{
		case DepthComparisonFunction::LESS:
			return D3D12_COMPARISON_FUNC_LESS;
			break;
		case DepthComparisonFunction::LESS_EQUAL:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		case DepthComparisonFunction::GREATER:
			return D3D12_COMPARISON_FUNC_GREATER;
			break;
		case DepthComparisonFunction::GREATER_EQUAL:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			break;
		case DepthComparisonFunction::EQUAL:
			return D3D12_COMPARISON_FUNC_EQUAL;
			break;
		case DepthComparisonFunction::NOT_EQUAL:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			break;
		case DepthComparisonFunction::NEVER:
			return D3D12_COMPARISON_FUNC_NEVER;
			break;
		case DepthComparisonFunction::ALWAYS:
			return D3D12_COMPARISON_FUNC_ALWAYS;
			break;
		}

		RLS_ASSERT(false, "Unknown comparison function type encountered");
		return D3D12_COMPARISON_FUNC_NEVER;
	}

	Pipeline::Pipeline(const PipelineSpecification& specification) noexcept
		: m_Specification{specification}
	{
		RLS_ASSERT(specification.pVertexShader, "No valid Vertex Shader present in Pipeline Specification.");
		Initialize(specification);
	}

	std::shared_ptr<Pipeline> Pipeline::Create(const PipelineSpecification& specification) noexcept
	{
		return std::make_shared<Pipeline>(specification);
	}

	void Pipeline::Initialize(const PipelineSpecification& specification) noexcept
	{
		const FrameBufferSpecification& frameBufferSpecification = specification.pFrameBuffer->GetSpecification();

		//We need a rasterizer descriptor:
		D3D12_RASTERIZER_DESC rasterizerDescriptor = {};
		rasterizerDescriptor.FillMode = (specification.FillMode == FillMode::Solid) ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
		rasterizerDescriptor.CullMode = specification.BackfaceCulling ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
		rasterizerDescriptor.FrontCounterClockwise = FALSE;
		rasterizerDescriptor.DepthBias = 0;
		rasterizerDescriptor.DepthBiasClamp = 0.0f;
		rasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
		rasterizerDescriptor.DepthClipEnable = TRUE;
		rasterizerDescriptor.MultisampleEnable = frameBufferSpecification.MSAASamples > 1 ? TRUE : FALSE;
		rasterizerDescriptor.AntialiasedLineEnable = frameBufferSpecification.MSAASamples > 1 ? TRUE : FALSE;
		rasterizerDescriptor.ForcedSampleCount = 0u;
		rasterizerDescriptor.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		//We also need a Stream Output Descriptor:
		D3D12_STREAM_OUTPUT_DESC streamOutputDescriptor = {};
		streamOutputDescriptor.pSODeclaration = nullptr;
		streamOutputDescriptor.NumEntries = 0u;
		streamOutputDescriptor.pBufferStrides = nullptr;
		streamOutputDescriptor.NumStrides = 0u;
		streamOutputDescriptor.RasterizedStream = 0u;

		//And a depth stencil descriptor:
		D3D12_DEPTH_STENCIL_DESC depthStencilDescriptor = {};
		depthStencilDescriptor.DepthEnable = (frameBufferSpecification.Attachments.DepthAttachment.Format != TextureFormat::None) ? TRUE : FALSE;
		depthStencilDescriptor.DepthWriteMask = specification.DepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilDescriptor.DepthFunc = RLSComparisonFuncToD3D12ComparisonFunc(frameBufferSpecification.DepthComparisonFunction);
		depthStencilDescriptor.StencilEnable = FALSE;
		depthStencilDescriptor.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilDescriptor.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		depthStencilDescriptor.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		depthStencilDescriptor.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		depthStencilDescriptor.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor{};
		psoDescriptor.VS.pShaderBytecode = specification.pVertexShader->GetBuffer()->GetBufferPointer();
		psoDescriptor.VS.BytecodeLength = specification.pVertexShader->GetBuffer()->GetBufferSize();
		psoDescriptor.PS.pShaderBytecode = specification.pPixelShader ? specification.pPixelShader->GetBuffer()->GetBufferPointer() : nullptr;
		psoDescriptor.PS.BytecodeLength = specification.pPixelShader ? specification.pPixelShader->GetBuffer()->GetBufferSize() : 0u;
		psoDescriptor.RasterizerState = rasterizerDescriptor;

		for (uint32_t i{ 0u }; i < frameBufferSpecification.Attachments.ColorAttachments.size(); ++i)
		{
			psoDescriptor.RTVFormats[i] = RLSTextureFormatToDXGITextureFormat(frameBufferSpecification.Attachments.ColorAttachments[i].Format);

			if (frameBufferSpecification.Attachments.ColorAttachments[i].Blend)
			{
				D3D12_RENDER_TARGET_BLEND_DESC blendDescriptor = {};
				blendDescriptor.BlendEnable = TRUE;
				blendDescriptor.LogicOpEnable = FALSE;
				blendDescriptor.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				blendDescriptor.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				blendDescriptor.BlendOp = D3D12_BLEND_OP_ADD;
				blendDescriptor.SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDescriptor.DestBlendAlpha = D3D12_BLEND_ONE;
				blendDescriptor.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				blendDescriptor.LogicOp = D3D12_LOGIC_OP_NOOP;
				blendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

				psoDescriptor.BlendState.RenderTarget[i] = blendDescriptor;
			}
			else
			{
				D3D12_RENDER_TARGET_BLEND_DESC blendDescriptor = {};
				blendDescriptor.BlendEnable = FALSE;
				blendDescriptor.LogicOpEnable = FALSE;
				blendDescriptor.SrcBlend = D3D12_BLEND_ONE;
				blendDescriptor.DestBlend = D3D12_BLEND_ZERO;
				blendDescriptor.BlendOp = D3D12_BLEND_OP_ADD;
				blendDescriptor.SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDescriptor.DestBlendAlpha = D3D12_BLEND_ZERO;
				blendDescriptor.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				blendDescriptor.LogicOp = D3D12_LOGIC_OP_NOOP;
				blendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

				psoDescriptor.BlendState.RenderTarget[i] = blendDescriptor;
			}
		}

		psoDescriptor.BlendState.AlphaToCoverageEnable = false;
		psoDescriptor.BlendState.IndependentBlendEnable = false;
		psoDescriptor.NumRenderTargets = static_cast<UINT>(frameBufferSpecification.Attachments.ColorAttachments.size());
		psoDescriptor.SampleMask = UINT_MAX;
		psoDescriptor.PrimitiveTopologyType = (specification.Topology == Topology::Triangle) ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
			: (specification.Topology == Topology::Line) ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

		psoDescriptor.SampleDesc.Quality = 0u;
		psoDescriptor.SampleDesc.Count = frameBufferSpecification.MSAASamples;
		psoDescriptor.StreamOutput = streamOutputDescriptor;
		psoDescriptor.DepthStencilState = depthStencilDescriptor;
		psoDescriptor.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
		psoDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		m_ReflectionResult = ShaderReflector::Reflect(specification.pVertexShader, specification.pPixelShader);
		psoDescriptor.pRootSignature = m_ReflectionResult.pRootSignature.Get();

		DXCall(D3D12Core::GetDevice()->CreateGraphicsPipelineState(&psoDescriptor, IID_PPV_ARGS(&m_pPSO)));
		NAME_D12_OBJECT(m_pPSO, ConvertStringToWstring(specification.DebugName).c_str());
	}

	void Pipeline::OnMSAAReconfiguration(uint8_t nrOfSamples) noexcept
	{
		if (m_Specification.MSAAEligible)
		{
			m_Specification.pFrameBuffer->OnMSAAReconfiguration(nrOfSamples);
			Initialize(m_Specification);
		}
		else 
		{
			m_Specification.pFrameBuffer->SynchronizeDependencies();
		}
	}
}