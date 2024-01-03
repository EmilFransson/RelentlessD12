#include "Mesh.h"
#include "Scene/Scene.h"
#include "Utility/SerializeUtilities.h"
#include "Vertex.h"
#include "../../vendor/includes/Assimp/Importer.hpp"
#include "../../vendor/includes/Assimp/postprocess.h"

namespace Relentless
{
	inline static std::mutex g_LoadMutex2;

	Mesh::Mesh(const std::string& name, const VertexBuffer::Specification& vbSpec, const IndexBuffer::Specification& ibSpec) noexcept
		:m_Name{ name }
	{
		m_pVertexBuffer = std::make_unique<VertexBuffer>(&vbSpec);
		m_pIndexBuffer = std::make_unique<IndexBuffer>(&ibSpec);
	}

	Mesh::Mesh()
		: m_Name{"Unnamed"},
		  m_pVertexBuffer{nullptr},
		  m_pIndexBuffer{nullptr}
	{}

	Mesh::Mesh(const Mesh& otherMesh) noexcept
		: m_Name{ otherMesh.m_Name },
		  m_pVertexBuffer{ std::make_unique<VertexBuffer>(*otherMesh.m_pVertexBuffer)},
		  m_pIndexBuffer{ std::make_unique<IndexBuffer>(*otherMesh.m_pIndexBuffer)}
	{}

	Mesh::Mesh(Mesh&& otherMesh) noexcept
		: m_Name{std::move(otherMesh.m_Name)},
		  m_pVertexBuffer{std::move(otherMesh.m_pVertexBuffer)},
		  m_pIndexBuffer{std::move(otherMesh.m_pIndexBuffer)}
	{
		otherMesh.m_pVertexBuffer = nullptr;
		otherMesh.m_pIndexBuffer = nullptr;
	}

	Mesh& Mesh::operator=(Mesh&& otherMesh) noexcept
	{
		if (this != &otherMesh)
		{
			m_Name = std::move(otherMesh.m_Name);
			m_pVertexBuffer = std::move(otherMesh.m_pVertexBuffer);
			m_pIndexBuffer = std::move(otherMesh.m_pIndexBuffer);

			otherMesh.m_pVertexBuffer = nullptr;
			otherMesh.m_pIndexBuffer = nullptr;
		}

		return *this;
	}

	Mesh& Mesh::operator=(const Mesh& otherMesh) noexcept
	{
		if (this != &otherMesh)
		{
			m_Name = otherMesh.m_Name;
			m_pVertexBuffer = std::make_unique<VertexBuffer>(*otherMesh.m_pVertexBuffer);
			m_pIndexBuffer = std::make_unique<IndexBuffer>(*otherMesh.m_pIndexBuffer);
		}

		return *this;
	}

	void Mesh::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	void Mesh::SetVertexBuffer(std::unique_ptr<VertexBuffer>&& pVertexBuffer) noexcept
	{
		m_pVertexBuffer = std::move(pVertexBuffer);
	}

	void Mesh::SetIndexBuffer(std::unique_ptr<IndexBuffer>&& pIndexBuffer) noexcept
	{
		m_pIndexBuffer = std::move(pIndexBuffer);
	}
}