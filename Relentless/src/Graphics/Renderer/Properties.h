#pragma once
namespace Relentless
{
	enum class TextureFormat : uint8_t
	{
		None = 0u,
		RGBA32F,
		R32UINT,
		Depth
	};

	struct Attachments
	{
		TextureFormat ColorAttachment{ TextureFormat::None };
		TextureFormat DepthAttachment{ TextureFormat::None };
	};

	struct MSAASpecification
	{
		bool Enabled{ false };
		uint8_t Count{ 1u };
		uint8_t Quality{ 0u };
	};

	enum class FillMode : uint8_t
	{
		Solid = 0u,
		Wireframe
	};

	enum class Topology : uint8_t
	{
		Unknown = 0,
		Triangle,
		Line,
		Point
	};

	DXGI_FORMAT RLSTextureFormatToDXGITextureFormat(TextureFormat format) noexcept;
}