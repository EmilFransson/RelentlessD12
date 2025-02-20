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