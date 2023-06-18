#pragma once
#include "Properties.h"
#include "../Resources/Texture.h"
#include "../Resources/DepthStencil.h"
namespace Relentless
{
	struct FrameBufferSpecification
	{
		std::string DebugName = "?";
		uint32_t Width{800u};
		uint32_t Height{600u};
		MSAASpecification MSAA{};
		DirectX::XMFLOAT4 ClearColor{0.0f, 0.0f, 0.0f, 1.0f};
		Attachments Attachments{};
		bool Transfer{ false };
		D3D12_RESOURCE_FLAGS Flags{ D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET };
		bool IsSRGB{ true };
	};

	class FrameBuffer
	{
	public:
		explicit FrameBuffer(const FrameBufferSpecification& specification) noexcept;
		~FrameBuffer() noexcept = default;
		static [[nodiscard]] std::shared_ptr<FrameBuffer> Create(const FrameBufferSpecification& specification) noexcept;
		[[nodiscard]] const FrameBufferSpecification& GetSpecification() const noexcept { return m_Specification; }
		void Resize(uint32_t width, uint32_t height) noexcept;
		[[nodiscard]] const std::shared_ptr<RenderTexture>& GetColorBuffer() const noexcept { return m_pColorBuffer; }
		[[nodiscard]] const std::shared_ptr<DepthStencil>& GetDepthBuffer() const noexcept { return m_pDepthBuffer; }
	private:
		FrameBufferSpecification m_Specification;
		std::shared_ptr<RenderTexture> m_pColorBuffer;
		std::shared_ptr<DepthStencil> m_pDepthBuffer;
	};
}