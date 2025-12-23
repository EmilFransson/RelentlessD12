#pragma once

#include "Assets/AssetMeta.h"
#include "Core/IAsset.h"
#include "Math/MathTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class Mesh : public IAsset
	{
	public:
		Mesh(Ref<Buffer> pVertexBuffer, Ref<Buffer> pIndexBuffer, const std::string& name = "Unnamed") noexcept;
		
		void SetOffsetTransform(const Matrix& transform) noexcept;
		NO_DISCARD const Matrix& GetOffsetTransform() const noexcept;

		NO_DISCARD const AssetHandle& GetDefaultMaterialHandle() noexcept;
		NO_DISCARD Buffer* GetVertexBuffer() const noexcept;
		NO_DISCARD Buffer* GetIndexBuffer() const noexcept;

		void SetDefaultMaterial(const AssetHandle& handle) noexcept;
	private:
		Matrix m_OffsetTransform{};
		Ref<Buffer> m_pVertexBuffer = nullptr;
		Ref<Buffer> m_pIndexBuffer = nullptr;

		BoundingBox Bounds;

		AssetHandle m_DefaultMaterialHandle = NULL_HANDLE;
	};
}