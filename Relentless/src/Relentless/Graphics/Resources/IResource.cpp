#include "IResource.h"
namespace Relentless
{
	IResource::IResource() noexcept
		:m_pResource{ nullptr },
		 m_DescriptorHandle{},
		 m_CurrentState{D3D12_RESOURCE_STATE_COMMON}
	{
	}
}