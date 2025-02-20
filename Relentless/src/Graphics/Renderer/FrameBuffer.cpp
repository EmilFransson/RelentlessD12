#include "FrameBuffer.h"
#include "..\MemoryManager.h"
#include "Core/Application.h"

namespace Relentless
{
	FrameBuffer::FrameBuffer(const FrameBufferSpecification& specification) noexcept
		: m_Specification{specification}
	{
		for (uint32_t i{0u}; i < specification.Attachments.ColorAttachments.size(); ++i)
		{
			RLS_ASSERT(specification.Attachments.ColorAttachments[i].Format != TextureFormat::None, "Attachment format is invalid.");

			if (specification.Attachments.ColorAttachments[i].Output)
			{
				//We already have an output, meaning it is dependent on another framebuffer from which it originally stems. 
				RLS_ASSERT(specification.Attachments.ColorAttachments[i].pOutputDependency, "Frame buffer dependency is invalid for color attachment.");

				//Update attachment details with information given through the already created Color Output:
				m_Specification.Attachments.ColorAttachments[i].ClearColor = specification.Attachments.ColorAttachments[i].Output->GetClearColor();
				m_Specification.Width = specification.Attachments.ColorAttachments[i].Output->GetWidth();
				m_Specification.Height = specification.Attachments.ColorAttachments[i].Output->GetHeight();
				m_Specification.MSAASamples = specification.Attachments.ColorAttachments[i].Output->GetMultiSampleCount();

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
			rtSpec.MultiSampleCount = specification.MSAASamples;
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
		
		//RLS_ASSERT(specification.Attachments.DepthAttachment.Format == TextureFormat::Depth, "Invalid depth attachment format.");
		
		//By this point we know a depth output should be created:
		DepthStencilSpecification depthStencilSpecification;
		depthStencilSpecification.Width = specification.Width;
		depthStencilSpecification.Height = specification.Height;
		depthStencilSpecification.Format = specification.Attachments.DepthAttachment.Format == TextureFormat::Depth ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_R32_TYPELESS;
		depthStencilSpecification.Samples = specification.MSAASamples;
		depthStencilSpecification.CreateSRV = specification.Attachments.DepthAttachment.Transfer ? true : false;
		m_Specification.Attachments.DepthAttachment.Output = DepthStencil::Create(depthStencilSpecification, specification.DebugName + " - DepthStencil");
	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& specification) noexcept
	{
		return std::make_shared<FrameBuffer>(specification);
	}

	void FrameBuffer::Resize(uint32_t width, uint32_t height) noexcept
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		//auto& memoryManager = Application::Get().GetMemorymanager();
		for (uint32_t i{ 0u }; i < m_Specification.Attachments.ColorAttachments.size(); ++i)
		{
			if (m_Specification.Attachments.ColorAttachments[i].ShouldResize)
			{
				if (m_Specification.Attachments.ColorAttachments[i].Format != TextureFormat::None)
				{
					if ((m_Specification.Attachments.ColorAttachments[i].Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0)
					{
						//memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.ColorAttachments[i].Output->GetRTVDescriptorHandle());
					}
					if (m_Specification.Attachments.ColorAttachments[i].Transfer)
					{
						//memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.ColorAttachments[i].Output->GetSRVDescriptorHandle());
					}
					//memoryManager.DestroyResource(std::move(m_Specification.Attachments.ColorAttachments[i].Output));

					RenderTextureSpecification rtSpec{};
					rtSpec.ClearColor = m_Specification.Attachments.ColorAttachments[i].ClearColor;
					rtSpec.CreateSRV = m_Specification.Attachments.ColorAttachments[i].Transfer ? true : false;
					rtSpec.Width = width;
					rtSpec.Height = height;
					rtSpec.Format = RLSTextureFormatToDXGITextureFormat(m_Specification.Attachments.ColorAttachments[i].Format);
					rtSpec.MultiSampleCount = m_Specification.MSAASamples;
					rtSpec.Flags = m_Specification.Attachments.ColorAttachments[i].Flags;
					rtSpec.isSRGB = m_Specification.Attachments.ColorAttachments[i].IsSRGB;

					m_Specification.Attachments.ColorAttachments[i].Output = RenderTexture::Create(rtSpec, m_Specification.DebugName + " - RenderTexture[" + std::to_string(i) + "]");
				}
			}
		}

		if (m_Specification.Attachments.DepthAttachment.Format != TextureFormat::None)
		{
			if (m_Specification.Attachments.DepthAttachment.ShouldResize)
			{
				//memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.DepthAttachment.Output->GetDSVDescriptorHandle());
				if (m_Specification.Attachments.DepthAttachment.Transfer)
				{
					//memoryManager.DestroyDescriptorHandle(m_Specification.Attachments.DepthAttachment.Output->GetSRVDescriptorHandle());
				}
				
				//memoryManager.DestroyResource(std::move(m_Specification.Attachments.DepthAttachment.Output));

				DepthStencilSpecification depthStencilSpecification;
				depthStencilSpecification.Width = width;
				depthStencilSpecification.Height = height;
				depthStencilSpecification.Format = RLSTextureFormatToDXGITextureFormat(m_Specification.Attachments.DepthAttachment.Format);
				depthStencilSpecification.Samples = m_Specification.MSAASamples;
				depthStencilSpecification.CreateSRV = m_Specification.Attachments.DepthAttachment.Transfer ? true : false;
				m_Specification.Attachments.DepthAttachment.Output = DepthStencil::Create(depthStencilSpecification, m_Specification.DebugName + " - DepthStencil");
			}
		}
		SynchronizeDependencies();
	}

	void FrameBuffer::OnMSAAReconfiguration(uint8_t nrOfSamples) noexcept
	{
		m_Specification.MSAASamples = nrOfSamples;
		Resize(m_Specification.Width, m_Specification.Height);
	}

	void FrameBuffer::SynchronizeDependencies() noexcept
	{
		for (uint32_t i{ 0u }; i < m_Specification.Attachments.ColorAttachments.size(); ++i)
		{
			if (m_Specification.Attachments.ColorAttachments[i].pOutputDependency)
			{
				m_Specification.Attachments.ColorAttachments[i].Output = m_Specification.Attachments.ColorAttachments[i].pOutputDependency->GetOutput(i);
			}
		}
		if (m_Specification.Attachments.DepthAttachment.pOutputDependency)
		{
			m_Specification.Attachments.DepthAttachment.Output = m_Specification.Attachments.DepthAttachment.pOutputDependency->GetDepthOutput();;
		}

		//Just update the dimensions according to output in slot 0.
		//m_Specification.Width = m_Specification.Attachments.ColorAttachments[0].Output->GetWidth();
		//m_Specification.Height = m_Specification.Attachments.ColorAttachments[0].Output->GetHeight();
	}

	void FrameBuffer::SetOutputDependency(std::shared_ptr<FrameBuffer> dependency, uint32_t outputSlot) noexcept
	{
		RLS_ASSERT(!(outputSlot > m_Specification.Attachments.ColorAttachments.size()), "Output dependency slot is invalid.");

		m_Specification.Attachments.ColorAttachments[outputSlot].pOutputDependency = dependency;
	}

	void FrameBuffer::SetDepthDependency(std::shared_ptr<FrameBuffer> dependency) noexcept
	{
		m_Specification.Attachments.DepthAttachment.pOutputDependency = dependency;
	}

}