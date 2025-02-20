#include "PipelineState.h"

#include "Device.h"
#include "D3D.h"
#include "RootSignature.h"

namespace Relentless
{
	PipelineState::PipelineState(GraphicsDevice* pDevice, const PipelineStateInitializer& pipelineStateInitializer) noexcept
		: DeviceObject(pDevice), m_Desc{ pipelineStateInitializer }
	{}

	PipelineState::~PipelineState() noexcept
	{
		if (m_pPipelineState)
			GetParent()->DeferReleaseObject(m_pPipelineState.Detach());
	}

	void PipelineState::ConditionallyReload() noexcept
	{
		if (m_NeedsReload || !m_pPipelineState)
			CreateInternal();
	}

	ID3D12PipelineState* PipelineState::GetPipelineState() const noexcept
	{
		return m_pPipelineState;
	}

	uint32 PipelineState::GetSampleCount() noexcept
	{
		const DXGI_SAMPLE_DESC& desc = m_Desc.m_Stream.SampleDesc;
		return desc.Count;
	}

	void PipelineState::CreateInternal() noexcept
	{
		std::lock_guard guard(m_BuildMutex);
		if (!m_NeedsReload)
			return;

		if (m_pPipelineState)
			GetParent()->DeferReleaseObject(m_pPipelineState.Detach());

		/*
			Shaders
		*///TODO! ADJUST SHADER LIBRARY TO NEW VERSION!!!
		ShaderLibrary* pShaderLibrary = GetParent()->GetShaderLibrary();
		for (int i = 0; i < m_Desc.m_ShaderInfo.size(); ++i)
		{
			const std::string& shaderName = m_Desc.m_ShaderInfo[i];
			if (shaderName.empty())
				continue;

			const std::shared_ptr<Shader> pShader = pShaderLibrary->Get(shaderName);
			const ShaderType shaderType = (ShaderType)i;
			switch (shaderType)
			{
			case ShaderType::VERTEX:
			{
				D3D12_SHADER_BYTECODE& shaderByteCode = m_Desc.m_Stream.VS;
				shaderByteCode.pShaderBytecode = pShader->GetBuffer()->GetBufferPointer();
				shaderByteCode.BytecodeLength = pShader->GetBuffer()->GetBufferSize();
				break;
			}
			case ShaderType::PIXEL:
			{
				D3D12_SHADER_BYTECODE& shaderByteCode = m_Desc.m_Stream.PS;
				shaderByteCode.pShaderBytecode = pShader->GetBuffer()->GetBufferPointer();
				shaderByteCode.BytecodeLength = pShader->GetBuffer()->GetBufferSize();
				break;
			}
			case ShaderType::Compute:
			{
				D3D12_SHADER_BYTECODE& shaderByteCode = m_Desc.m_Stream.CS;
				shaderByteCode.pShaderBytecode = pShader->GetBuffer()->GetBufferPointer();
				shaderByteCode.BytecodeLength = pShader->GetBuffer()->GetBufferSize();
				break;
			}
			default:
				RLS_ASSERT(false, "Unreachable.");
			}
		}

		/*
			Blending
		*/
		auto&& AreBlendStatesEqual = [](const D3D12_RENDER_TARGET_BLEND_DESC& baseline, const D3D12_RENDER_TARGET_BLEND_DESC& comparator) -> bool
		{
			return 
				baseline.BlendEnable == comparator.BlendEnable &&
				baseline.BlendOp == comparator.BlendOp &&
				baseline.BlendOpAlpha == comparator.BlendOpAlpha &&
				baseline.DestBlend == comparator.DestBlend &&
				baseline.DestBlendAlpha == comparator.DestBlendAlpha &&
				baseline.LogicOp == comparator.LogicOp &&
				baseline.LogicOpEnable == comparator.LogicOpEnable &&
				baseline.RenderTargetWriteMask == comparator.RenderTargetWriteMask &&
				baseline.SrcBlend == comparator.SrcBlend &&
				baseline.SrcBlendAlpha == comparator.SrcBlendAlpha;
		};

		const D3D12_RT_FORMAT_ARRAY& formatArray = m_Desc.m_Stream.RTFormats;
		CD3DX12_BLEND_DESC& blendDesc = m_Desc.m_Stream.Blend;

		const D3D12_RENDER_TARGET_BLEND_DESC& baseline = blendDesc.RenderTarget[0];
		bool blendModesIdentical = true;

		for (uint8 i = 1; i < formatArray.NumRenderTargets; ++i)
		{
			if (!AreBlendStatesEqual(baseline, blendDesc.RenderTarget[i]))
			{
				blendModesIdentical = false;
				break;
			}
		}

		blendDesc.IndependentBlendEnable = !blendModesIdentical;
		
		/*
			Build
		*/
		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
		streamDesc.SizeInBytes = sizeof(m_Desc.m_Stream);
		streamDesc.pPipelineStateSubobjectStream = &m_Desc.m_Stream;

		VERIFY_HR_EX(GetParent()->GetDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(m_pPipelineState.ReleaseAndGetAddressOf())), GetParent()->GetDevice());
		RLS_ASSERT(m_pPipelineState, "[PipelineState::CreateInternal] Error Creating Pipeline State.");

		RLS_CORE_INFO("Created Pipeline State: {0}", m_Desc.m_Name.c_str());
		D3D::SetObjectName(m_pPipelineState, m_Desc.m_Name.c_str());
		m_NeedsReload = false;
	}

	PipelineStateInitializer::PipelineStateInitializer() noexcept
	{
		m_Stream.Blend = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		m_Stream.DepthStencil = CD3DX12_DEPTH_STENCIL_DESC1(D3D12_DEFAULT);
		m_Stream.Rasterizer = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		m_Stream.SampleDesc = DXGI_SAMPLE_DESC{ 1, 0 };
		m_Stream.SampleMask = 0xFFFFFFFF;
		m_Stream.PrimitiveTopology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		m_Stream.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	}

	void PipelineStateInitializer::SetName(const char* pName) noexcept
	{
		m_Name = pName;
	}

	void PipelineStateInitializer::SetDepthOnlyTarget(ResourceFormat dsvFormat, uint32 msaa) noexcept
	{
		SetRenderTargetFormats({}, dsvFormat, msaa);
	}

	void PipelineStateInitializer::SetRenderTargetFormats(Span<ResourceFormat> rtvFormats, ResourceFormat dsvFormat, uint32 msaa) noexcept
	{
		D3D12_RT_FORMAT_ARRAY& formatArray = m_Stream.RTFormats;
		// Validation layer bug - Throws error about RT Format even if NumRenderTargets == 0.
		memset(formatArray.RTFormats, 0, sizeof(DXGI_FORMAT) * ARRAYSIZE(formatArray.RTFormats));
		formatArray.NumRenderTargets = 0;
		for (ResourceFormat format : rtvFormats)
		{
			formatArray.RTFormats[formatArray.NumRenderTargets++] = D3D::ConvertFormat(format);
		}

		DXGI_SAMPLE_DESC& sampleDesc = m_Stream.SampleDesc;
		sampleDesc.Count = msaa;
		sampleDesc.Quality = 0;

		D3D12_RASTERIZER_DESC& rasterDesc = m_Stream.Rasterizer;
		rasterDesc.MultisampleEnable = msaa > 1;

		m_Stream.DSVFormat = D3D::ConvertFormat(dsvFormat);
	}

	void PipelineStateInitializer::SetBlendMode(BlendMode blendMode, uint8 renderTargetIndex) noexcept
	{
		D3D12_BLEND_DESC& blendDesc = m_Stream.Blend;
		D3D12_RENDER_TARGET_BLEND_DESC& desc = blendDesc.RenderTarget[renderTargetIndex];
		desc.RenderTargetWriteMask = 0xf; 
		desc.BlendEnable = blendMode == BlendMode::Replace ? false : true;

		switch (blendMode)
		{
		case BlendMode::Replace:
			desc.SrcBlend = D3D12_BLEND_ONE;
			desc.DestBlend = D3D12_BLEND_ZERO;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_ONE;
			desc.DestBlendAlpha = D3D12_BLEND_ZERO;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::Alpha:
			desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::Additive:
			desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlend = D3D12_BLEND_ONE;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_ONE;
			desc.DestBlendAlpha = D3D12_BLEND_ONE;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::Multiply:
			desc.SrcBlend = D3D12_BLEND_DEST_COLOR;
			desc.DestBlend = D3D12_BLEND_ZERO;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_DEST_COLOR;
			desc.DestBlendAlpha = D3D12_BLEND_ZERO;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::AddAlpha:
			desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlend = D3D12_BLEND_ONE;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlendAlpha = D3D12_BLEND_ONE;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::PreMultiplyAlpha:
			desc.SrcBlend = D3D12_BLEND_ONE;
			desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_ONE;
			desc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::InverseDestinationAlpha:
			desc.SrcBlend = D3D12_BLEND_INV_DEST_ALPHA;
			desc.DestBlend = D3D12_BLEND_DEST_ALPHA;
			desc.BlendOp = D3D12_BLEND_OP_ADD;
			desc.SrcBlendAlpha = D3D12_BLEND_INV_DEST_ALPHA;
			desc.DestBlendAlpha = D3D12_BLEND_DEST_ALPHA;
			desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;
		case BlendMode::Subtract:
			desc.SrcBlend = D3D12_BLEND_ONE;
			desc.DestBlend = D3D12_BLEND_ONE;
			desc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			desc.SrcBlendAlpha = D3D12_BLEND_ONE;
			desc.DestBlendAlpha = D3D12_BLEND_ONE;
			desc.BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
			break;
		case BlendMode::SubtractAlpha:
			desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlend = D3D12_BLEND_ONE;
			desc.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			desc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			desc.DestBlendAlpha = D3D12_BLEND_ONE;
			desc.BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
			break;
		case BlendMode::Undefined:
		default:
			break;
		}
	}

	void PipelineStateInitializer::SetAlphaToCoverageEnable(bool enabled) noexcept
	{
		D3D12_BLEND_DESC& blendDesc = m_Stream.Blend;
		blendDesc.AlphaToCoverageEnable = enabled;
	}

	void PipelineStateInitializer::SetLineAntiAliasingEnabled(bool enabled) noexcept
	{
		CD3DX12_RASTERIZER_DESC& rasterizerDesc = m_Stream.Rasterizer;
		rasterizerDesc.AntialiasedLineEnable = enabled;
	}

	void PipelineStateInitializer::SetFillMode(D3D12_FILL_MODE fillMode) noexcept
	{
		CD3DX12_RASTERIZER_DESC& rasterizerDesc = m_Stream.Rasterizer;
		rasterizerDesc.FillMode = fillMode;
	}

	void PipelineStateInitializer::SetCullMode(D3D12_CULL_MODE cullMode) noexcept
	{
		CD3DX12_RASTERIZER_DESC& rasterizerDesc = m_Stream.Rasterizer;
		rasterizerDesc.CullMode = cullMode;
	}

	void PipelineStateInitializer::SetFrontCounterClockWise(bool enabled) noexcept
	{
		CD3DX12_RASTERIZER_DESC& rasterizerDesc = m_Stream.Rasterizer;
		rasterizerDesc.FrontCounterClockwise = enabled;
	}

	void PipelineStateInitializer::SetDepthBias(int depthBias, float depthBiasClamp, float slopeScaledDepthBias) noexcept
	{
		CD3DX12_RASTERIZER_DESC& rasterizerDesc = m_Stream.Rasterizer;
		rasterizerDesc.DepthBias = depthBias;
		rasterizerDesc.DepthBiasClamp = depthBiasClamp;
		rasterizerDesc.SlopeScaledDepthBias = slopeScaledDepthBias;
	}

	void PipelineStateInitializer::SetDepthEnabled(bool enabled) noexcept
	{
		CD3DX12_DEPTH_STENCIL_DESC1& depthStencilDesc = m_Stream.DepthStencil;
		depthStencilDesc.DepthEnable = enabled;
	}

	void PipelineStateInitializer::SetDepthWrite(bool enabled) noexcept
	{
		CD3DX12_DEPTH_STENCIL_DESC1& depthStencilDesc = m_Stream.DepthStencil;
		depthStencilDesc.DepthWriteMask = enabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	}

	void PipelineStateInitializer::SetDepthFunc(D3D12_COMPARISON_FUNC comparisonFunc) noexcept
	{
		CD3DX12_DEPTH_STENCIL_DESC1& depthStencilDesc = m_Stream.DepthStencil;
		depthStencilDesc.DepthFunc = comparisonFunc;
	}

	void PipelineStateInitializer::SetStencilEnabled(bool enabled) noexcept
	{
		CD3DX12_DEPTH_STENCIL_DESC1& depthStencilDesc = m_Stream.DepthStencil;
		depthStencilDesc.StencilEnable = enabled;
	}

	void PipelineStateInitializer::SetStencilTest(bool stencilEnabled, D3D12_COMPARISON_FUNC mode, D3D12_STENCIL_OP pass, D3D12_STENCIL_OP fail, D3D12_STENCIL_OP zFail, unsigned char compareMask, unsigned char writeMask) noexcept
	{
		CD3DX12_DEPTH_STENCIL_DESC1& depthStencilDesc = m_Stream.DepthStencil;
		depthStencilDesc.StencilEnable = stencilEnabled;
		depthStencilDesc.FrontFace.StencilFunc = mode;
		depthStencilDesc.FrontFace.StencilPassOp = pass;
		depthStencilDesc.FrontFace.StencilFailOp = fail;
		depthStencilDesc.FrontFace.StencilDepthFailOp = zFail;
		depthStencilDesc.StencilReadMask = compareMask;
		depthStencilDesc.StencilWriteMask = writeMask;
		depthStencilDesc.BackFace = depthStencilDesc.FrontFace;
	}

	void PipelineStateInitializer::SetVertexShader(const char* pShaderName) noexcept
	{
		m_ShaderInfo[(int)ShaderType::VERTEX] = pShaderName;
	}

	void PipelineStateInitializer::SetPixelShader(const char* pShaderName) noexcept
	{
		m_ShaderInfo[(int)ShaderType::PIXEL] = pShaderName;
	}

	void PipelineStateInitializer::SetComputeShader(const char* pShaderName) noexcept
	{
		m_ShaderInfo[(int)ShaderType::Compute] = pShaderName;
	}

	void PipelineStateInitializer::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) noexcept
	{
		m_Stream.PrimitiveTopology = topology;
	}

	void PipelineStateInitializer::SetRootSignature(RootSignature* pRootSignature) noexcept
	{
		m_Stream.pRootSignature = pRootSignature->GetRootSignature();
	}
}