#pragma once

#include "Assets/AssetMeta.h"
#include "Core/IAsset.h"
#include "Math/MathTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class Mesh : public IAsset, public RefCounted<Mesh>
	{
	public:
		explicit Mesh(const std::string& name = "Unnamed") noexcept;
		Mesh(Ref<Buffer> pVertexBuffer, Ref<Buffer> pIndexBuffer, const std::string& name = "Unnamed") noexcept;
		
		Mesh(const Mesh& otherMesh) noexcept;
		Mesh& operator=(const Mesh& otherMesh) noexcept;
		Mesh(Mesh&& otherMesh) noexcept;
		Mesh& operator=(Mesh&& otherMesh) noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(const std::string& name) noexcept;
		void SetOffsetTransform(const Transform& transform) noexcept;
		[[nodiscard]] const Transform& GetOffsetTransform() const noexcept;

		[[nodiscard]] const AssetHandle& GetDefaultMaterialHandle() noexcept;
		[[nodiscard]] Buffer* GetVertexBuffer() const noexcept;
		[[nodiscard]] Buffer* GetIndexBuffer() const noexcept;

		void SetDefaultMaterial(const AssetHandle& handle) noexcept;
	private:
		std::string m_Name;
		Transform m_OffsetTransform{};
		Ref<Buffer> m_pVertexBuffer = nullptr;
		Ref<Buffer> m_pIndexBuffer = nullptr;

		BoundingBox Bounds;

		AssetHandle m_DefaultMaterialHandle = NULL_HANDLE;
	};
}