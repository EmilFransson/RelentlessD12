#include "IResource.h"
namespace Relentless
{
	IResource::IResource(const std::string& name) noexcept
		:m_pResource{ nullptr },
		 m_Name{name},
		 m_CurrentState{D3D12_RESOURCE_STATE_COMMON}
	{
	}
}