#pragma once
#include "D3D.h"
#include "DeviceResource.h"
#include "Core/CoreTypes.h"
#include "Core/IAsset.h"

namespace Relentless
{
	enum class TextureFlag : uint8
	{
		None			= 0,
		ShaderResource	= 1 << 0,
		UnorderedAccess = 1 << 1,
		RenderTarget	= 1 << 2,
		DepthStencil	= 1 << 3,
		sRGB			= 1 << 4
	};
	DECLARE_BITMASK_TYPE(TextureFlag);

	enum class TextureType : uint8
	{
		Texture2D,
		TextureCube
	};

	struct ClearBinding
	{
		struct DepthStencilData
		{
			DepthStencilData(float depth = 0.0f, uint8 stencil = 1u) noexcept
				: Depth{depth}, Stencil(stencil)
			{
			}

			float Depth		= 0.0f;
			uint8 Stencil	= 1u;
		};

		enum class ClearBindingValue : uint8
		{
			None,
			Color,
			DepthStencil
		};

		ClearBinding() noexcept
			: BindingValue{ClearBindingValue::None}, DepthStencil{DepthStencilData()}
		{}

		ClearBinding(Color color) noexcept
			: BindingValue{ClearBindingValue::Color}, Color{color}
		{}

		ClearBinding(float depth, uint8 stencil) noexcept
			: BindingValue{ClearBindingValue::DepthStencil}, DepthStencil{depth, stencil}
		{}

		bool operator==(const ClearBinding& other) const
		{
			if (BindingValue != other.BindingValue)
			{
				return false;
			}
			if (BindingValue == ClearBindingValue::Color)
			{
				return Color == other.Color;
			}
			return DepthStencil.Depth == other.DepthStencil.Depth
				&& DepthStencil.Stencil == other.DepthStencil.Stencil;
		}

		ClearBindingValue BindingValue = ClearBindingValue::None;
		union
		{
			Color Color;
			DepthStencilData DepthStencil;
		};
	};

	struct TextureDesc
	{
		uint32			Width				: 14	= 1;
		uint32			Height				: 14	= 1;
		uint32			DepthOrArraySize	: 10	= 1;
		uint32			Mips				: 5		= 1;
		uint32			SampleCount			: 3		= 1;
		TextureType		Type						= TextureType::Texture2D;
		TextureFlag		Flags						= TextureFlag::None;
		ResourceFormat	Format						= ResourceFormat::Unknown;
		ClearBinding	ClearBindingValue			= ClearBinding(Colors::Black);

		bool operator==(const TextureDesc&) const = default;

		static [[nodiscard]] TextureDesc CreateCube(uint32 width, uint32 height, ResourceFormat format, uint32 mips = 1u, TextureFlag flags = TextureFlag::None, const ClearBinding& clearBinding = ClearBinding(Colors::Black), uint32 sampleCount = 1) noexcept
		{
			RLS_ASSERT(width > 0u, "Width is invalid.");
			RLS_ASSERT(height > 0u, "Height is invalid.");

			TextureDesc desc;
			desc.Width = width;
			desc.Height = height;
			desc.DepthOrArraySize = 1u;
			desc.Mips = mips;
			desc.SampleCount = sampleCount;
			desc.Type = TextureType::TextureCube;
			desc.Flags = flags;
			desc.Format = format;
			desc.ClearBindingValue = clearBinding;
			return desc;
		}

		static [[nodiscard]] TextureDesc Create2D(uint32 width, uint32 height, ResourceFormat format, uint32 mips = 1u, TextureFlag flags = TextureFlag::None, const ClearBinding& clearBinding = ClearBinding(Colors::Black), uint32 sampleCount = 1) noexcept
		{
			RLS_ASSERT(width > 0u, "Width is invalid.");
			RLS_ASSERT(height > 0u, "Height is invalid.");

			TextureDesc desc;
			desc.Width = width;
			desc.Height = height;
			desc.DepthOrArraySize = 1u;
			desc.Mips = mips;
			desc.SampleCount = sampleCount;
			desc.Type = TextureType::Texture2D;
			desc.Flags = flags;
			desc.Format = format;
			desc.ClearBindingValue = clearBinding;
			return desc;
		}

		bool IsCompatible(const TextureDesc& other) const
		{
			return Width == other.Width
				&& Height == other.Height
				&& DepthOrArraySize == other.DepthOrArraySize
				&& Mips == other.Mips
				&& SampleCount == other.SampleCount
				&& Format == other.Format
				&& ClearBindingValue == other.ClearBindingValue
				&& Type == other.Type
				&& EnumHasAllFlags(Flags, other.Flags);
		}
	};

	class Texture : public DeviceResource
	{
	public:
		Texture(GraphicsDevice* pParent, const TextureDesc& desc, ID3D12Resource2* pResource) noexcept;

		[[nodiscard]] uint32 GetWidth() const noexcept;
		[[nodiscard]] uint32 GetHeight() const noexcept;
		[[nodiscard]] uint32 GetArraySize() const noexcept;
		[[nodiscard]] uint32 GetMipLevels() const noexcept;
		[[nodiscard]] uint32 GetSampleCount() const noexcept;
		[[nodiscard]] TextureType GetType() const noexcept;
		[[nodiscard]] ResourceFormat GetFormat() const noexcept;
		[[nodiscard]] const ClearBinding& GetClearBinding() const noexcept;
		[[nodiscard]] const TextureDesc& GetDesc() const noexcept;

		[[nodiscard]] DepthStencilView* GetDSV(uint32 subResourceIndex = 0) const noexcept;
		[[nodiscard]] uint32 GetDSVIndex(uint32 subResourceIndex = 0) const noexcept;

		[[nodiscard]] ShaderResourceView* GetSRV() const noexcept;
		[[nodiscard]] uint32 GetSRVIndex() const noexcept;

		[[nodiscard]] UnorderedAccessView* GetUAV(uint32 subResourceIndex = 0) const noexcept;
		[[nodiscard]] uint32 GetUAVIndex(uint32 subResourceIndex = 0) const noexcept;

		[[nodiscard]] RenderTargetView* GetRTV(uint32 subResourceIndex = 0) const noexcept;
		[[nodiscard]] uint32 GetRTVIndex(uint32 subResourceIndex = 0) const noexcept;

		void SetDSV(Ref<DepthStencilView> pDSV, uint32 subResourceIndex = 0u) noexcept;
		void SetSRV(Ref<ShaderResourceView> pSRV) noexcept;
		void SetRTV(Ref<RenderTargetView> pRTV, uint32 subResourceIndex = 0u) noexcept;
		void SetUAV(Ref<UnorderedAccessView> pUAV, uint32 subResourceIndex = 0u) noexcept;

	private:
		const TextureDesc m_Desc;

		Ref<ShaderResourceView> m_pSRV = nullptr;
		std::vector<Ref<UnorderedAccessView>> m_UAVs;
		std::vector<Ref<RenderTargetView>> m_RTVs;
		std::vector<Ref<DepthStencilView>> m_DSVs;
	};
}