#pragma once
#include "Graphics/Resources/IndexBuffer.h"
#include "Graphics/Resources/VertexBuffer.h"
#include "Graphics/Resources/ResourceMeta.h"
#include "Math/MathTypes.h"

namespace Relentless
{
	class BufferEx;
	
	class Mesh : public RefCounted<Mesh>
	{
	public:
		explicit Mesh(const std::string& name = "Unnamed") noexcept;
		Mesh(Ref<BufferEx> pVertexBuffer, Ref<BufferEx> pIndexBuffer, const std::string& name = "Unnamed") noexcept;
		
		Mesh(const Mesh& otherMesh) noexcept;
		Mesh& operator=(const Mesh& otherMesh) noexcept;
		Mesh(Mesh&& otherMesh) noexcept;
		Mesh& operator=(Mesh&& otherMesh) noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(const std::string& name) noexcept;
		void SetVertexBufferHandle(ResourceHandle handle) noexcept;
		void SetIndexBufferHandle(ResourceHandle handle) noexcept;
		void SetOffsetTransform(const Transform& transform) noexcept;
		[[nodiscard]] const Transform& GetOffsetTransform() const noexcept;

		[[nodiscard]] BufferEx* GetVertexBuffer() const noexcept;
		[[nodiscard]] BufferEx* GetIndexBuffer() const noexcept;

		//ResourceHandle m_VertexBufferHandle = NULL_RESOURCE_HANDLE;
		//ResourceHandle m_IndexBufferHandle = NULL_RESOURCE_HANDLE;
	private:
		std::string m_Name;
		Transform m_OffsetTransform{};
		Ref<BufferEx> m_pVertexBuffer = nullptr;
		Ref<BufferEx> m_pIndexBuffer = nullptr;
	};
}