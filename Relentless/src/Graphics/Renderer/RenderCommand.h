#pragma once
namespace Relentless
{
	class IResource;
	class DescriptorHeap;
	class Texture;
	class RenderTexture;
	class ReadBackBuffer;
	class DepthStencil;
	struct BackBuffer;
	class RenderCommand
	{
	public:
	private:
		RenderCommand() noexcept = delete;
		~RenderCommand() noexcept = default;
	};
}