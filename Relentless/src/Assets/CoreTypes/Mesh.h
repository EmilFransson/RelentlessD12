#pragma once
#include "Assets/AssetMeta.h"
#include "Assets/IAsset.h"

#include "Core/DLLExport.h"

#include "Math/MathTypes.h"

#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class Material;

	class RLS_API Mesh : public AssetBase<Mesh>
	{
	public:
		Mesh() noexcept;
		explicit Mesh(const UUID& aUUID) noexcept;
		Mesh(Ref<Buffer> aVertexBuffer, Ref<Buffer> aIndexBuffer, const String& aName = "Unnamed") noexcept;
		virtual ~Mesh() noexcept override;
		
		NO_DISCARD Ref<Material> GetDefaultMaterial() noexcept;
		NO_DISCARD const AssetHandle& GetDefaultMaterialHandle() noexcept;
		NO_DISCARD const Matrix& GetOffsetTransform() const noexcept;
		NO_DISCARD Buffer* GetVertexBuffer() const noexcept;
		NO_DISCARD Buffer* GetIndexBuffer() const noexcept;

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0x599f0573, 0x4a11, 0x42a4, { 0x8c, 0x8d, 0x66, 0x26, 0xd5, 0x82, 0x28, 0x58 } };
			return uid;
		}

		void SetDefaultMaterial(const AssetHandle& aMaterialHandle) noexcept;
		void SetOffsetTransform(const Matrix& aOffsetTransform) noexcept;
		bool SerializeCore(IArchive& aArchive) noexcept override;
		bool SerializeBulk(IArchive& aArchive) noexcept override;
	private:
		Matrix m_OffsetTransform = Matrix::Identity;
		BoundingBox m_Bounds;
		AssetHandle m_DefaultMaterialHandle = AssetHandle::INVALID;
		
		Ref<Buffer> m_pVertexBuffer = nullptr;
		Ref<Buffer> m_pIndexBuffer = nullptr;
	};
}