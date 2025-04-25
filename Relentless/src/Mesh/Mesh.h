#pragma once
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
		void SetOffsetTransform(const Transform& transform) noexcept;
		[[nodiscard]] const Transform& GetOffsetTransform() const noexcept;

		[[nodiscard]] BufferEx* GetVertexBuffer() const noexcept;
		[[nodiscard]] BufferEx* GetIndexBuffer() const noexcept;
	private:
		std::string m_Name;
		Transform m_OffsetTransform{};
		Ref<BufferEx> m_pVertexBuffer = nullptr;
		Ref<BufferEx> m_pIndexBuffer = nullptr;
	};
}