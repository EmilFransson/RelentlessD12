#include "FrameBuffer.h"
#include "..\MemoryManager.h"

namespace Relentless
{
	FrameBuffer::FrameBuffer(const FrameBufferSpecification& specification) noexcept
		: m_Specification{specification}
	{
		for (uint32_t i{0u}; i < specification.Attachments.ColorAttachments.size(); ++i)
		{
			//We already have an output, meaning it is dependent on another framebuffer from which it originally stems. 
			if (specification.Attachments.ColorAttachments[i].Output)
			{
				RLS_ASSERT(specification.Attachments.ColorAttachments[i].pOutputDependency, "Frame buffer dependency is invalid for color attachment.");

				//Update attachment details with information given through the already created Color Output:
				m_Specification.Attachments.ColorAttachments[i].ClearColor = specification.Attachments.ColorAttachments[i].Output->GetClearColor();
				m_Specification.Width = specification.Attachments.ColorAttachments[i].Output->GetWidth();
				m_Specification.Height = specification.Attachments.ColorAttachments[i].Output->GetHeight();
				m_Specification.MSAA.Count = specification.Attachments.ColorAttachments[i].Output->GetMultiSampleCount();
				m_Specification.MSAA.Quality = 0u;
				m_Specification.MSAA.Enabled = m_Specification.MSAA.Count > 1 ? true : false;

				continue;
			}

			// We do not have an output already, and also we would NOT like to create one,
			// as no format has been set.
			if (specification.Attachments.ColorAttachments[i].Format == TextureFormat::None)
			{
				continue;
			}

			//By this point the tests have concluded that a new output (RenderTexture) should be created.
			if (specification.Attachments.ColorAttachments[i].IsSRGB)
			{
				DirectX::XMVECTOR convertedColor = DirectX::XMColorSRGBToRGB(DirectX::XMLoadFloat4(&specification.Attachments.ColorAttachments[i].ClearColor));
				m_Specification.Attachments.ColorAttachments[i].ClearColor.x = DirectX::XMVectorGetX(convertedColor);
				m_Specification.Attachments.ColorAttachments[i].ClearColor.y = DirectX::XMVectorGetY(convertedColor);
				m_Specification.Attachments.ColorAttachments[i].ClearColor.z = DirectX::XMVectorGetZ(convertedColor);
				m_Specification.Attachments.ColorAttachments[i].ClearColor.w = DirectX::XMVectorGetW(convertedColor);
			}

			RenderTextureSpecification rtSpec{};
			rtSpec.ClearColor = specification.Attachments.ColorAttachments[i].ClearColor;
			rtSpec.CreateSRV = specification.Attachments.ColorAttachments[i].Transfer ? true : false;
			rtSpec.Width = specification.Width;
			rtSpec.Height = specification.Height;
			rtSpec.Format = RLSTextureFormatToDXGITextureFormat(specification.Attachments.ColorAttachments[i].Format);
			rtSpec.MultiSampleCount = specification.MSAA.Count;
			rtSpec.Flags = specification.Attachments.ColorAttachments[i].Flags;
			rtSpec.isSRGB = false; //Handled by frame buffer

			m_Specification.Attachments.ColorAttachments[i].Output = RenderTexture::Create(rtSpec, specification.DebugName + " - RenderTexture[" + std::to_string(i) + "]");
		}

		if (specification.Attachments.DepthAttachment.Output)
		{
			//A depth attachment has already been provided, so no new should be created. 
			RLS_ASSERT(specification.Attachments.DepthAttachment.pOutputDependency, "Frame buffer dependency is invalid for depth attachment.");
			return;
		}
		if (specification.Attachments.DepthAttachment.Format == TextureFormat::None)
		{
			//None should be created
			return;
		}
		
		RLS_ASSERT(specification.Attachments.DepthAttachment.Format == TextureFormat::Depth, "Invalid depth attachment format.");
		
		//By this point we know a depth output should be created:
		m_Specification.Attachments.DepthAttachment.Output = DepthStencil::Create(specification.Width, specification.Height, specification.MSAA.Count, specification.DebugName + " - DepthStencil");
	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& specification) noexcept
	{
		return std::make_shared<FrameBuffer>(specification);
	}

	void FrameBuffer::Resize(uint32_t width, uint32_t height) noexcept
	{
		if (m_Specification.ShouldResize)
		{
			auto& memoryManager = MemoryManager::Get();
			if (m_Specification.Attachments.ColorAttachments[0].Format != TextureFormat::None)
			{
				if ((m_Specification.Attachments.ColorAttachments[0].Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0)
				{
					memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.ColorAttachments[0].Output->GetRTVDescriptorHandle());
				}
				if (m_Specification.Attachments.ColorAttachments[0].Transfer)
				{
					memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.ColorAttachments[0].Output->GetSRVDescriptorHandle());
				}
				memoryManager.DestroyResource(std::move(m_Specification.Attachments.ColorAttachments[0].Output));

				RenderTextureSpecification rtSpec{};
				rtSpec.ClearColor = m_Specification.Attachments.ColorAttachments[0].ClearColor;
				rtSpec.CreateSRV = m_Specification.Attachments.ColorAttachments[0].Transfer ? true : false;
				rtSpec.Width = width;
				rtSpec.Height = height;
				rtSpec.Format = RLSTextureFormatToDXGITextureFormat(m_Specification.Attachments.ColorAttachments[0].Format);
				rtSpec.MultiSampleCount = m_Specification.MSAA.Count;
				rtSpec.Flags = m_Specification.Attachments.ColorAttachments[0].Flags;
				rtSpec.isSRGB = m_Specification.Attachments.ColorAttachments[0].IsSRGB;

				m_Specification.Attachments.ColorAttachments[0].Output = RenderTexture::Create(rtSpec, m_Specification.DebugName + " - RenderTexture[" + std::to_string(0) + "]");
			}

			if (m_Specification.Attachments.DepthAttachment.Format != TextureFormat::None)
			{
				memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.DepthAttachment.Output->GetDSVDescriptorHandle());
				memoryManager.DestroyResource(std::move(m_Specification.Attachments.DepthAttachment.Output));
				m_Specification.Attachments.DepthAttachment.Output = DepthStencil::Create(width, height, m_Specification.MSAA.Count, m_Specification.DebugName + " - DepthStencil");
			}

			m_Specification.Width = width;
			m_Specification.Height = height;
		}
		else
		{
			if (m_Specification.Attachments.ColorAttachments[0].pOutputDependency)
			{
				auto colorDependency = m_Specification.Attachments.ColorAttachments[0].pOutputDependency->GetOutput(0);
				m_Specification.Attachments.ColorAttachments[0].Output = colorDependency;

			}
			if (m_Specification.Attachments.DepthAttachment.pOutputDependency)
			{
				auto depthDependency = m_Specification.Attachments.DepthAttachment.pOutputDependency->GetDepthOutput();
				m_Specification.Attachments.DepthAttachment.Output= depthDependency;
			}

			m_Specification.Width = width;
			m_Specification.Height = height;
		}
	}
}