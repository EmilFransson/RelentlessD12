#pragma once

namespace Relentless
{
	class BufferEx;
	struct BufferDesc;
	struct BufferSRVDesc;
	class CommandContext;
	class CommandQueue;
	class DepthStencilView;
	class DescriptorHeapEx;
	class DeviceObject;
	class Fence;
	class GraphicsDevice;
	class PipelineState;
	class PipelineStateInitializer;
	class RenderTargetView;
	class RingBufferAllocator;
	class RootSignature;
	class ScratchAllocationManager;
	class ShaderResourceView;
	class Swapchain;
	class TextureEx;
	struct TextureDesc;
	struct TextureDSVDesc;
	struct TextureRTVDesc;
	struct TextureSRVDesc;
	struct TextureUAVDesc;
	class UnorderedAccessView;

	enum class DepthTargetAccessFlags : uint8;

	enum class ResourceFormat : uint8 
	{
		Unknown = 0u,

		R8_UINT,
		R8_SINT,
		R8_UNORM,
		R8_SNORM,
		RG8_UINT,
		RG8_SINT,
		RG8_UNORM,
		RG8_SNORM,
		R16_UINT,
		R16_SINT,
		R16_UNORM,
		R16_SNORM,
		R16_FLOAT,
		BGRA4_UNORM,
		B5G6R5_UNORM,
		B5G5R5A1_UNORM,
		RGBA8_UINT,
		RGBA8_SINT,
		RGBA8_UNORM,
		RGBA8_SNORM,
		BGRA8_UNORM,
		RGB10A2_UNORM,
		R11G11B10_FLOAT,
		RG16_UINT,
		RG16_SINT,
		RG16_UNORM,
		RG16_SNORM,
		RG16_FLOAT,
		R32_UINT,
		R32_SINT,
		R32_FLOAT,
		R32_TYPELESS,
		RGBA16_UINT,
		RGBA16_SINT,
		RGBA16_FLOAT,
		RGBA16_UNORM,
		RGBA16_SNORM,
		RG32_UINT,
		RG32_SINT,
		RG32_FLOAT,
		RGB32_UINT,
		RGB32_SINT,
		RGB32_FLOAT,
		RGBA32_UINT,
		RGBA32_SINT,
		RGBA32_FLOAT,

		BC1_UNORM,
		BC2_UNORM,
		BC3_UNORM,
		BC4_UNORM,
		BC4_SNORM,
		BC5_UNORM,
		BC5_SNORM,
		BC6H_UFLOAT,
		BC6H_SFLOAT,
		BC7_UNORM,

		D16_UNORM,
		D32_FLOAT,
		D24S8,
		D32S8,

		RGBA8_UNORM_SRGB,
		BGRA8_UNORM_SRGB,

		Count
	};

	enum class FormatType : uint8
	{
		Integer,
		Normalized,
		Float,
		DepthStencil,
		Typeless
	};

	struct FormatInfo
	{
		const char*		pName;
		ResourceFormat	Format;
		FormatType		Type;
		uint8			BytesPerBlock	: 8;
		uint8			BlockSize		: 4;
		uint8			NumComponents	: 3;
		uint8			IsDepth			: 1;
		uint8			IsStencil		: 1;
		uint8			IsSigned		: 1;
		uint8			IsBC			: 1;
	};

	namespace RHI
	{
		[[nodiscard]] const FormatInfo& GetFormatInfo(ResourceFormat format) noexcept;

		[[nodiscard]] uint64 GetRowPitch(ResourceFormat format, uint32 width, uint32 mipIndex = 0) noexcept;
		[[nodiscard]] uint64 GetSlicePitch(ResourceFormat format, uint32 width, uint32 height, uint32 mipIndex = 0) noexcept;
		[[nodiscard]] uint64 GetTextureMipByteSize(ResourceFormat format, uint32 width, uint32 height, uint32 depth, uint32 mipIndex) noexcept;
		[[nodiscard]] uint64 GetTextureByteSize(ResourceFormat format, uint32 width, uint32 height, uint32 depth = 1, uint32 numMips = 1) noexcept;
	}
}