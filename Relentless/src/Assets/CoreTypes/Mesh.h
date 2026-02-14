#pragma once
#include "Assets/AssetMeta.h"
#include "Assets/IAsset.h"

#include "Core/DLLExport.h"

#include "Math/MathTypes.h"

#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class RLS_API Mesh : public AssetBase<Mesh>
	{
	public:
		Mesh() noexcept;
		explicit Mesh(const UUID& aUUID) noexcept;
		Mesh(Ref<Buffer> pVertexBuffer, Ref<Buffer> pIndexBuffer, const std::string& name = "Unnamed") noexcept;
		virtual ~Mesh() noexcept override;
		
		void SetOffsetTransform(const Matrix& transform) noexcept;
		NO_DISCARD const Matrix& GetOffsetTransform() const noexcept;

		NO_DISCARD const AssetHandle& GetDefaultMaterialHandle() noexcept;
		NO_DISCARD Buffer* GetVertexBuffer() const noexcept;
		NO_DISCARD Buffer* GetIndexBuffer() const noexcept;

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0x599f0573, 0x4a11, 0x42a4, { 0x8c, 0x8d, 0x66, 0x26, 0xd5, 0x82, 0x28, 0x58 } };
			return uid;
		}

		void SetDefaultMaterial(const AssetHandle& handle) noexcept;
		bool SerializeCore(IArchive& aArchive) noexcept override;
		bool SerializeBulk(IArchive& aArchive) noexcept override;
	private:
		Matrix m_OffsetTransform{};
		Ref<Buffer> m_pVertexBuffer = nullptr;
		Ref<Buffer> m_pIndexBuffer = nullptr;

		BoundingBox m_Bounds;

		AssetHandle m_DefaultMaterialHandle = AssetHandle::INVALID;
	};
}