#pragma once
#include "DeviceResource.h"
#include "Graphics/Shaders/Shader.h"

namespace Relentless
{
	enum class BlendMode
	{
		Replace = 0,
		Additive,
		Multiply,
		Alpha,
		AddAlpha,
		PreMultiplyAlpha,
		InverseDestinationAlpha,
		Subtract,
		SubtractAlpha,
		Undefined,
	};

	class PipelineStateInitializer
	{
		friend class PipelineState;
	public:
		PipelineStateInitializer() noexcept;

		void SetName(const char* pName) noexcept;
		void SetDepthOnlyTarget(ResourceFormat dsvFormat, uint32 msaa) noexcept;
		void SetRenderTargetFormats(std::span<ResourceFormat> rtvFormats, ResourceFormat dsvFormat, uint32 msaa) noexcept;

		//Blend:
		void SetBlendMode(BlendMode blendMode, uint8 renderTargetIndex = 0u) noexcept;
		void SetAlphaToCoverageEnable(bool enabled) noexcept;

		//Rasterizer:
		void SetLineAntiAliasingEnabled(bool enabled) noexcept;
		void SetFillMode(D3D12_FILL_MODE fillMode) noexcept;
		void SetDepthBias(int depthBias, float depthBiasClamp, float slopeScaledDepthBias) noexcept;
		void SetCullMode(D3D12_CULL_MODE cullMode) noexcept;
		void SetFrontCounterClockWise(bool enabled) noexcept;

		//Depth Stencil:
		void SetDepthEnabled(bool enabled) noexcept;
		void SetDepthWrite(bool enabled) noexcept;
		void SetDepthFunc(D3D12_COMPARISON_FUNC comparisonFunc) noexcept;
		void SetStencilEnabled(bool enabled) noexcept;
		void SetStencilTest(bool stencilEnabled, D3D12_COMPARISON_FUNC mode, D3D12_STENCIL_OP pass, D3D12_STENCIL_OP fail, D3D12_STENCIL_OP zFail, unsigned char compareMask, unsigned char writeMask) noexcept;
	
		//Shaders:
		void SetVertexShader(const char* pShaderName) noexcept;
		void SetPixelShader(const char* pShaderName) noexcept;
		void SetComputeShader(const char* pShaderName) noexcept;

		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) noexcept;
		void SetRootSignature(RootSignature* pRootSignature) noexcept;
	private:
#pragma warning(push)
#pragma warning(disable : 4324)
		template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE SubObjectType, typename T>
		struct alignas(void*) StreamSubObject
		{
			StreamSubObject() = default;

			StreamSubObject(const T& rhs)
				: Type(SubObjectType), InnerObject(rhs)
			{}
			operator T& () { return InnerObject; }

		private:
			D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type = SubObjectType;
			T InnerObject{};
		};
#pragma warning(pop)
		struct ObjectStream
		{
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE> VS;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, D3D12_SHADER_BYTECODE> PS;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS, D3D12_SHADER_BYTECODE> CS;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, D3D12_SHADER_BYTECODE> AS;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, D3D12_SHADER_BYTECODE> MS;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS, D3D12_RT_FORMAT_ARRAY> RTFormats;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT, DXGI_FORMAT> DSVFormat;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1, CD3DX12_DEPTH_STENCIL_DESC1> DepthStencil;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER, CD3DX12_RASTERIZER_DESC> Rasterizer;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND, CD3DX12_BLEND_DESC> Blend;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY, D3D12_PRIMITIVE_TOPOLOGY_TYPE> PrimitiveTopology;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT, D3D12_INPUT_LAYOUT_DESC> InputLayout;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*> pRootSignature;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK, UINT> SampleMask;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC, DXGI_SAMPLE_DESC> SampleDesc;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE, D3D12_INDEX_BUFFER_STRIP_CUT_VALUE> StripCutValue;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT, D3D12_STREAM_OUTPUT_DESC> StreamOutput;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS, D3D12_PIPELINE_STATE_FLAGS> Flags;
			StreamSubObject<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK, UINT> NodeMask;
		} m_Stream;

		std::string m_Name;
		std::array<std::string, (size_t)ShaderType::Max> m_ShaderInfo;
	};

	class PipelineState : public DeviceObject
	{
	public:
		PipelineState(GraphicsDevice* pDevice, const PipelineStateInitializer& pipelineStateInitializer) noexcept;
		virtual ~PipelineState() noexcept;

		void ConditionallyReload() noexcept;

		[[nodiscard]] ID3D12PipelineState* GetPipelineState() const noexcept;
	private:
		friend class GraphicsDevice;
		void CreateInternal() noexcept;
	private:
		PipelineStateInitializer m_Desc;
		Ref<ID3D12PipelineState> m_pPipelineState = nullptr;
		bool m_NeedsReload = true;
		std::mutex m_BuildMutex;
	};
}