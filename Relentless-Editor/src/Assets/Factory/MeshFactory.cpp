#include "MeshFactory.h"

namespace Relentless
{
	bool MeshFactory::CanCreateNew() const noexcept
	{
		return true;
	}

	bool MeshFactory::CanImport(MAYBE_UNUSED const Path& aPath) const noexcept
	{
		return false;
	}

	Ref<IFactory> MeshFactory::Clone() noexcept
	{
		return RLS_NEW MeshFactory();
	}

	FactoryCreateResult MeshFactory::CreateNew(const String& aName, const UUID& aUUID) noexcept
	{
		Ref<Mesh> pNewMesh = RLS_NEW Mesh(aUUID);
		pNewMesh->SetName(aName);

		return pNewMesh;
	}

	bool MeshFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Mesh::StaticType();
	}

	String MeshFactory::GetDefaultNewAssetName() const noexcept
	{
		return "NewMesh";
	}

	String MeshFactory::GetDisplayName() const noexcept
	{
		return "MeshFactory";
	}

	bool MeshFactory::ShouldShowInNewMenu() const noexcept
	{
		return false;
	}
}