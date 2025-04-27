#include "Mesh.h"
#include "Graphics/RHI/Buffer.h"

namespace Relentless
{
	Mesh::Mesh(const std::string& name) noexcept
		:m_Name{ name }
	{}

	Mesh::Mesh(const Mesh& otherMesh) noexcept
		: m_Name{ otherMesh.m_Name }//,
	{}

	Mesh::Mesh(Mesh&& otherMesh) noexcept
		: m_Name{std::move(otherMesh.m_Name)}//,
	{
	}

	Mesh::Mesh(Ref<Buffer> pVertexBuffer, Ref<Buffer> pIndexBuffer, const std::string& name /*= "Unnamed"*/) noexcept
		: m_pVertexBuffer{ pVertexBuffer }, m_pIndexBuffer{ pIndexBuffer }, m_Name{ name }
	{
	}

	Mesh& Mesh::operator=(Mesh&& otherMesh) noexcept
	{
		if (this != &otherMesh)
		{
			m_Name = std::move(otherMesh.m_Name);
		}

		return *this;
	}

	Mesh& Mesh::operator=(const Mesh& otherMesh) noexcept
	{
		if (this != &otherMesh)
		{
			m_Name = otherMesh.m_Name;
		}

		return *this;
	}

	void Mesh::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	void Mesh::SetOffsetTransform(const Transform& transform) noexcept
	{
		m_OffsetTransform = transform;
	}

	const Transform& Mesh::GetOffsetTransform() const noexcept
	{
		return m_OffsetTransform;
	}

	const Relentless::AssetHandle& Mesh::GetDefaultMaterialHandle() noexcept
	{
		return m_DefaultMaterialHandle;
	}

	Buffer* Mesh::GetVertexBuffer() const noexcept
	{
		RLS_ASSERT(m_pVertexBuffer, "[Mesh::GetVertexBuffer] Vertex Buffer Is Invalid.");
		return m_pVertexBuffer;
	}

	Buffer* Mesh::GetIndexBuffer() const noexcept
	{
		RLS_ASSERT(m_pIndexBuffer, "[Mesh::GetIndexBuffer] Index Buffer Is Invalid.");
		return m_pIndexBuffer;
	}

	void Mesh::SetDefaultMaterial(const AssetHandle& handle) noexcept
	{
		m_DefaultMaterialHandle = handle;
	}

}