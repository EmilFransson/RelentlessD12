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