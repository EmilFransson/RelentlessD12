#pragma once
#include "Mesh.h"
#include "../Graphics/Resources/Helper.h"
#include "../Utility/ManagerUtilities.h"
namespace Relentless
{
	class ModelSerializer
	{
	public:
		static void Serialize(std::vector<MeshHandle> meshHandles, const std::string& path, const std::string& toReplace = "") noexcept;
		static void Deserialize(const std::string& fullPath, const std::shared_ptr<Scene> pScene = nullptr) noexcept;
	};
}