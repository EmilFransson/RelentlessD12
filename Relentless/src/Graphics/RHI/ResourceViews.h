#pragma once
#include "DeviceResource.h"
#include "DescriptorHeap.h"
#include "CommandContext.h"

namespace Relentless
{
	class ResourceView : public DeviceObject
	{
	public:
		ResourceView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle) noexcept;
		virtual ~ResourceView() noexcept override;

		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const noexcept;
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const noexcept;
		[[nodiscard]] const DescriptorHandleEx& GetDescriptorHandle() const noexcept;
		[[nodiscard]] uint32_t GetDescriptorIndex() const noexcept;
	private:
		DescriptorHandleEx m_DescriptorHandle;
	};

	class ShaderResourceView : public ResourceView
	{
	public:
		ShaderResourceView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle) noexcept;
		virtual ~ShaderResourceView() noexcept override = default;
	};

	class UnorderedAccessView : public ResourceView
	{
	public:
		UnorderedAccessView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle) noexcept;
		virtual ~UnorderedAccessView() noexcept override = default;
	};

	class RenderTargetView : public ResourceView
	{
	public:
		RenderTargetView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle) noexcept;
		virtual ~RenderTargetView() noexcept override = default;
	};

	class DepthStencilView : public ResourceView
	{
	public:
		DepthStencilView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle) noexcept;
		virtual ~DepthStencilView() noexcept override = default;
	};

	struct VertexBufferView : public ResourceView
	{
		VertexBufferView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle, uint32 elements, uint32 stride, uint64 offsetFromStart = 0u) noexcept
			: ResourceView(pParent, descriptorHandle), Elements(elements), Stride(stride), OffsetFromStart((uint32)offsetFromStart)
		{
			RLS_ASSERT(offsetFromStart <= std::numeric_limits<uint32>::max(), "Buffer offset ({0}) will be stored in a 32-bit uint and does not fit.", offsetFromStart);
		}

		bool IsValid() const noexcept { return Elements > 0; }

		uint32 Elements			= 0u;
		uint32 Stride			= 0u;
		uint32 OffsetFromStart	= 0u;
	};

	struct IndexBufferView : public ResourceView
	{
		IndexBufferView(GraphicsDevice* pParent, const DescriptorHandleEx& descriptorHandle, [[maybe_unused]] D3D12_GPU_VIRTUAL_ADDRESS location, uint32 elements, ResourceFormat format, uint64 offsetFromStart) noexcept
			: ResourceView(pParent, descriptorHandle), Elements(elements), OffsetFromStart((uint32)offsetFromStart), Format(format)
		{
			RLS_ASSERT(offsetFromStart <= std::numeric_limits<uint32>::max(), "Buffer offset ({0}) will be stored in a 32-bit uint and does not fit.", offsetFromStart);
		}

		[[nodiscard]] uint32 Stride() const noexcept
		{
			return RHI::GetFormatInfo(Format).BytesPerBlock;
		}

		uint32 Elements			= 0u;
		uint32 OffsetFromStart	= 0u;
		ResourceFormat Format	= ResourceFormat::Unknown;
	};

	struct TextureDSVDesc
	{
		TextureDSVDesc(DepthTargetAccessFlags flags, uint32 mipSlice = 0u, uint32 firstArraySlice = 0u, uint32 arraySize = 1u) noexcept
			: Flags{ flags }, MipSlice{ mipSlice }, FirstArraySlice{ firstArraySlice }, ArraySize{ arraySize }
		{}

		uint32 MipSlice					= 0u;
		uint32 FirstArraySlice			= 0u;
		uint32 ArraySize				= 1u;
		DepthTargetAccessFlags Flags	= DepthTargetAccessFlags::None;

		bool operator==(const TextureDSVDesc& otherDesc) const
		{
			return MipSlice == otherDesc.MipSlice && FirstArraySlice == otherDesc.FirstArraySlice 
				&& ArraySize == otherDesc.ArraySize && EnumHasAllFlags(Flags, otherDesc.Flags);
		}
	};

	struct BufferSRVDesc
	{
		BufferSRVDesc(ResourceFormat format = ResourceFormat::Unknown, bool raw = false, uint32 elementOffset = 0, uint32 numElements = 0)
			: Format(format), Raw(raw), ElementOffset(elementOffset), NumElements(numElements)
		{
		}

		ResourceFormat Format	= ResourceFormat::Unknown;
		bool Raw				= false;
		uint32 ElementOffset	= 0u;
		uint32 NumElements		= 0u;
	};

	struct TextureSRVDesc
	{
		TextureSRVDesc(uint8 mipLevel, uint8 numMipLevels) noexcept
			: MipLevel{ mipLevel }, NumMipLevels{ numMipLevels }
		{}

		uint8 MipLevel		= 0u;
		uint8 NumMipLevels	= 0u;
		
		bool operator==(const TextureSRVDesc& otherDesc) const
		{
			return MipLevel == otherDesc.MipLevel && NumMipLevels == otherDesc.NumMipLevels;
		}
	};

	struct TextureRTVDesc
	{
		TextureRTVDesc(uint32 mipSlice = 0u, uint32 firstArraySlice = 0u, uint32 arraySize = 1u, uint32 planeSlice = 0u) noexcept
			: MipSlice{ mipSlice }, FirstArraySlice{ firstArraySlice }, ArraySize{ arraySize }, PlaneSlice{ planeSlice }
		{}

		uint32 MipSlice			= 0u;
		uint32 FirstArraySlice	= 0u;
		uint32 ArraySize		= 1u;
		uint32 PlaneSlice		= 0u;

		bool operator==(const TextureRTVDesc& otherDesc) const
		{
			return MipSlice == otherDesc.MipSlice && FirstArraySlice == otherDesc.FirstArraySlice 
				&& ArraySize == otherDesc.ArraySize && PlaneSlice == otherDesc.PlaneSlice;
		}
	};

	struct TextureUAVDesc
	{
		explicit TextureUAVDesc(uint8 mipLevel) noexcept
			: MipLevel(mipLevel)
		{}

		uint8 MipLevel;

		bool operator==(const TextureUAVDesc& other) const
		{
			return MipLevel == other.MipLevel;
		}
	};
}