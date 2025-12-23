#include "ResourceViews.h"
#include "Core/Application.h"
#include "Device.h"

namespace Relentless
{
	ResourceView::ResourceView(GraphicsDevice* pParent, const DescriptorHandle& descriptorHandle) noexcept
		:
		DeviceObject(pParent),
		m_DescriptorHandle(descriptorHandle)
	{}

	ResourceView::~ResourceView() noexcept
	{
		GetParent()->UnregisterGlobalDescriptor(m_DescriptorHandle);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ResourceView::GetCPUHandle() const noexcept
	{
		return m_DescriptorHandle.CPUHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE ResourceView::GetGPUHandle() const noexcept
	{
		return m_DescriptorHandle.GPUHandle;
	}

	const DescriptorHandle& ResourceView::GetDescriptorHandle() const noexcept
	{
		return m_DescriptorHandle;
	}

	uint32_t ResourceView::GetDescriptorIndex() const noexcept
	{
		return m_DescriptorHandle.Index;
	}

	ShaderResourceView::ShaderResourceView(GraphicsDevice* pParent, const DescriptorHandle& descriptorHandle) noexcept
		: ResourceView(pParent, descriptorHandle)
	{}

	UnorderedAccessView::UnorderedAccessView(GraphicsDevice* pParent, const DescriptorHandle& descriptorHandle, const DescriptorHandle& aCPUOpaqueDescriptorHandle) noexcept
		: ResourceView(pParent, descriptorHandle),
		  m_CPUOpaqueDescriptorHandle(aCPUOpaqueDescriptorHandle)
	{}

	D3D12_CPU_DESCRIPTOR_HANDLE UnorderedAccessView::GetCPUOpaqueHandle() const noexcept
	{
		return m_CPUOpaqueDescriptorHandle.CPUHandle;
	}

	RenderTargetView::RenderTargetView(GraphicsDevice* pParent, const DescriptorHandle& descriptorHandle) noexcept
		: ResourceView(pParent, descriptorHandle)
	{}

	DepthStencilView::DepthStencilView(GraphicsDevice* pParent, const DescriptorHandle& descriptorHandle) noexcept
		: ResourceView(pParent, descriptorHandle)
	{}

}