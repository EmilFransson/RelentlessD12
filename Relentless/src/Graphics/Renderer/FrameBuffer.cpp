#include "FrameBuffer.h"
#include "..\MemoryManager.h"

namespace Relentless
{

	FrameBuffer::FrameBuffer(const FrameBufferSpecification& specification) noexcept
		: m_Specification{specification},
		  m_pColorBuffer{nullptr},
		  m_pDepthBuffer{nullptr}
	{
		if (specification.Attachments.ColorAttachment != TextureFormat::None)
		{
			RenderTextureSpecification rtSpec{};
			rtSpec.ClearColor = specification.ClearColor;
			rtSpec.CreateSRV = specification.Transfer ? true : false;
			rtSpec.Width = specification.Width;
			rtSpec.Height = specification.Height;
			rtSpec.Format = RLSTextureFormatToDXGITextureFormat(specification.Attachments.ColorAttachment);
			rtSpec.MultiSampleCount = specification.MSAA.Count;
			rtSpec.Flags = specification.Flags;
			rtSpec.isSRGB = specification.IsSRGB;

			m_pColorBuffer = RenderTexture::Create(rtSpec, specification.DebugName + " - RenderTexture");
		}

		if (specification.Attachments.DepthAttachment != TextureFormat::None)
		{
			m_pDepthBuffer = DepthStencil::Create(specification.Width, specification.Height, specification.MSAA.Count, specification.DebugName + " - DepthStencil");
		}
	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& specification) noexcept
	{
		return std::make_shared<FrameBuffer>(specification);
	}

	void FrameBuffer::Resize(uint32_t width, uint32_t height) noexcept
	{
		RLS_ASSERT(width > 0 && height > 0, "Texture resize dimensions not valid.");

		auto& memoryManager = MemoryManager::Get();
		if (m_Specification.Attachments.ColorAttachment != TextureFormat::None)
		{
			if ((m_Specification.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0)
			{
				memoryManager.DestroyDescriptorHandle(m_pColorBuffer->GetRTVDescriptorHandle());
			}
			if (m_Specification.Transfer)
			{
				memoryManager.DestroyDescriptorHandle(m_pColorBuffer->GetSRVDescriptorHandle());
			}
			memoryManager.DestroyResource(std::move(m_pColorBuffer));

			RenderTextureSpecification rtSpec{};
			rtSpec.ClearColor = m_Specification.ClearColor;
			rtSpec.CreateSRV = m_Specification.Transfer ? true : false;
			rtSpec.Width = width;
			rtSpec.Height = height;
			rtSpec.Format = RLSTextureFormatToDXGITextureFormat(m_Specification.Attachments.ColorAttachment);
			rtSpec.MultiSampleCount = m_Specification.MSAA.Count;
			rtSpec.Flags = m_Specification.Flags;
			rtSpec.isSRGB = m_Specification.IsSRGB;

			m_pColorBuffer = RenderTexture::Create(rtSpec, m_Specification.DebugName + " - RenderTexture");
		}

		if (m_Specification.Attachments.DepthAttachment != TextureFormat::None)
		{
			memoryManager.DestroyDescriptorHandle(m_pDepthBuffer->GetDSVDescriptorHandle());
			memoryManager.DestroyResource(std::move(m_pDepthBuffer));
			m_pDepthBuffer = DepthStencil::Create(width, height, m_Specification.MSAA.Count, m_Specification.DebugName + " - DepthStencil");
		}
	}
}