#include "IResource.h"
namespace Relentless
{
	IResource::IResource(const std::string& name) noexcept
		:m_pResource{ nullptr },
		 m_Name{name},
		 m_CurrentState{D3D12_RESOURCE_STATE_COMMON}
	{
	}

	IResource::IResource(IResource&& otherResource) noexcept
		: m_pResource{ otherResource.m_pResource },
		  m_Name{ otherResource.m_Name },
		  m_CurrentState{ otherResource.m_CurrentState }
	{}

	IResource& IResource::operator=(IResource&& otherResource) noexcept
	{
		RLS_ASSERT(this != &otherResource, "Redundant move operation performed.");
		if (this != &otherResource)
		{
			m_Name = otherResource.m_Name;
			m_pResource = otherResource.m_pResource;
			m_CurrentState = otherResource.m_CurrentState;

			otherResource.m_pResource = nullptr;
		}
		return *this;
	}
}