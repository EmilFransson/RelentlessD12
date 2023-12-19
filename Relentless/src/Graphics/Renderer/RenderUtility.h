#pragma once
namespace Relentless
{
	class Texture;

	class RenderUtility
	{
	public:
		[[nodiscard]] static uint64_t GetTextureSizeInBytes(const std::shared_ptr<Texture>& pTexture) noexcept;
	};
}