#include "Mesh.h"
#include "Graphics/RHI/Buffer.h"

namespace Relentless
{
	Mesh::Mesh(const std::string& name) noexcept
		:m_Name{ name }
	{}

	Mesh::Mesh(const Mesh& otherMesh) noexcept
		: m_Name{ otherMesh.m_Name }//,
		  //m_VertexBufferHandle{ otherMesh.m_VertexBufferHandle },
		  //m_IndexBufferHandle{ otherMesh.m_IndexBufferHandle }
	{}

	Mesh::Mesh(Mesh&& otherMesh) noexcept
		: m_Name{std::move(otherMesh.m_Name)}//,
		  //m_VertexBufferHandle{ std::move(otherMesh.m_VertexBufferHandle) },
		  //m_IndexBufferHandle{std::move(otherMesh.m_IndexBufferHandle)}
	{
		//otherMesh.m_VertexBufferHandle = NULL_RESOURCE_HANDLE;
		//otherMesh.m_IndexBufferHandle = NULL_RESOURCE_HANDLE;
	}

	Mesh::Mesh(Ref<BufferEx> pVertexBuffer, Ref<BufferEx> pIndexBuffer, const std::string& name /*= "Unnamed"*/) noexcept
		: m_pVertexBuffer{ pVertexBuffer }, m_pIndexBuffer{ pIndexBuffer }, m_Name{ name }
	{
	}

	Mesh& Mesh::operator=(Mesh&& otherMesh) noexcept
	{
		if (this != &otherMesh)
		{
			m_Name = std::move(otherMesh.m_Name);
			//m_VertexBufferHandle = otherMesh.m_VertexBufferHandle;
			//m_IndexBufferHandle = otherMesh.m_IndexBufferHandle;
			//
			//otherMesh.m_VertexBufferHandle = NULL_RESOURCE_HANDLE;
			//otherMesh.m_IndexBufferHandle = NULL_RESOURCE_HANDLE;
		}

		return *this;
	}

	Mesh& Mesh::operator=(const Mesh& otherMesh) noexcept
	{
		if (this != &otherMesh)
		{
			m_Name = otherMesh.m_Name;
			//m_VertexBufferHandle = otherMesh.m_VertexBufferHandle;
			//m_IndexBufferHandle = otherMesh.m_IndexBufferHandle;
		}

		return *this;
	}

	void Mesh::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	void Mesh::SetVertexBufferHandle(ResourceHandle handle) noexcept
	{
		//m_VertexBufferHandle = handle;
	}

	void Mesh::SetIndexBufferHandle(ResourceHandle handle) noexcept
	{
		//m_IndexBufferHandle = handle;
	}

	void Mesh::SetOffsetTransform(const Transform& transform) noexcept
	{
		m_OffsetTransform = transform;
	}

	const Transform& Mesh::GetOffsetTransform() const noexcept
	{
		return m_OffsetTransform;
	}

	BufferEx* Mesh::GetVertexBuffer() const noexcept
	{
		RLS_ASSERT(m_pVertexBuffer, "[Mesh::GetVertexBuffer] Vertex Buffer Is Invalid.");
		return m_pVertexBuffer;
	}

	BufferEx* Mesh::GetIndexBuffer() const noexcept
	{
		RLS_ASSERT(m_pIndexBuffer, "[Mesh::GetIndexBuffer] Index Buffer Is Invalid.");
		return m_pIndexBuffer;
	}
}