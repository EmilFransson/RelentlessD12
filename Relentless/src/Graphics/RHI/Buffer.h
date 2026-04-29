#pragma once
#include "D3D.h"
#include "DeviceResource.h"
#include "Core/CoreTypes.h"

namespace Relentless
{
	enum class BufferFlag : uint8_t
	{
		None			= 0,
		ShaderResource	= 1 << 0,
		UnorderedAccess = 1 << 1,
		ReadBack		= 1 << 2,
		Upload			= 1 << 3,
		ByteAddress		= 1 << 4,
		NoBindless		= 1 << 5,
	};
	DECLARE_BITMASK_TYPE(BufferFlag);

	struct BufferDesc
	{
		uint64			Size			= 0u;
		uint32			ElementSize		= 1u;
		BufferFlag		Flags			= BufferFlag::None;
		ResourceFormat	Format			= ResourceFormat::Unknown;

		NO_DISCARD static BufferDesc CreateIndexBuffer(uint32_t elements, ResourceFormat format, BufferFlag flags = BufferFlag::None) noexcept
		{
			RLS_ASSERT(format == ResourceFormat::R32_UINT || format == ResourceFormat::R16_UINT, "Index Buffer Format Is Invalid.");
			const FormatInfo& formatInfo = RHI::GetFormatInfo(format);
			return { .Size = elements * formatInfo.BytesPerBlock, .ElementSize = formatInfo.BytesPerBlock, .Flags = flags };
		}

		NO_DISCARD static BufferDesc CreateVertexBuffer(uint32 elements, uint32 vertexSize, BufferFlag flags = BufferFlag::None) noexcept
		{
			return { .Size = elements * vertexSize, .ElementSize = vertexSize, .Flags = flags };
		}

		static BufferDesc CreateReadback(uint64 bytes) noexcept
		{
			RLS_ASSERT(bytes % 4 == 0, "[BufferDesc::CreateReadback] Invalid bytes");
			return { .Size = bytes, .ElementSize = 4, .Flags = BufferFlag::ReadBack | BufferFlag::NoBindless };
		}

		static BufferDesc CreateStructured(uint32 elementCount, uint32 elementSize, BufferFlag flags = BufferFlag::None)
		{
			return { .Size = (uint64)elementCount * elementSize, .ElementSize = elementSize, .Flags = flags | BufferFlag::ShaderResource };
		}

		static BufferDesc CreateTyped(uint32 elementCount, ResourceFormat format, BufferFlag flags = BufferFlag::None)
		{
			const FormatInfo& info = RHI::GetFormatInfo(format);
			RLS_ASSERT(!info.IsBC, "[BufferDesc::CreateTyped]: Block Compressed Format Is Unsupported.");
			return { .Size = (uint64)elementCount * info.BytesPerBlock, .ElementSize = info.BytesPerBlock, .Flags = flags | BufferFlag::ShaderResource, .Format = format };
		}

		NO_DISCARD uint32 NumElements() const noexcept
		{
			return static_cast<uint32>(Size / ElementSize);
		}
	};

	class RLS_API Buffer : public DeviceResource
	{
	public:
		Buffer(GraphicsDevice* pParent, const BufferDesc& desc, ID3D12Resource2* pResource) noexcept;
		virtual ~Buffer() noexcept override;

		NO_DISCARD const BufferDesc& GetDesc() const noexcept;
		NO_DISCARD BufferDesc& GetDesc() noexcept;
		NO_DISCARD void* GetMappedData() const noexcept;
		NO_DISCARD uint64 GetSize() const noexcept;
		NO_DISCARD uint64 GetNrOfElements() const noexcept;
		NO_DISCARD ShaderResourceView* GetSRV() const noexcept;
		NO_DISCARD uint32 GetSRVIndex() const noexcept;

		NO_DISCARD UnorderedAccessView* GetUAV() const noexcept;
		NO_DISCARD UnorderedAccessView* GetUAVNonVisible() const noexcept;
		NO_DISCARD uint32 GetUAVIndex() const noexcept;

		void Map(uint32 subresource, const D3D12_RANGE* pRange) noexcept;
		void Unmap(uint32 subresource, const D3D12_RANGE* pRange) noexcept;
		void SetSRV(Ref<ShaderResourceView> pSRV) noexcept;
		void SetUAV(Ref<UnorderedAccessView> pUAV) noexcept;
		void SetUAVNonVisible(Ref<UnorderedAccessView> pUAV) noexcept;
	private:
		BufferDesc m_Desc;
		void* m_pMappedPtr = nullptr;
		Ref<ShaderResourceView> m_pSRV = nullptr;
		Ref<UnorderedAccessView> m_pUAV = nullptr;
		Ref<UnorderedAccessView> m_pUAVNonVisible = nullptr;
	};
}