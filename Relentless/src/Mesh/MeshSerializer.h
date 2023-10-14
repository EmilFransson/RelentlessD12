#pragma once
#include "Mesh.h"
#include "../Graphics/Resources/Helper.h"
#include "../Utility/ManagerUtilities.h"
namespace Relentless
{
#pragma pack(push, 1)
	struct MeshDataHeader
	{
		uint32_t VertexBufferSizeInBytes;
		uint32_t IndexBufferSizeInBytes;
		uint32_t VertexCount;
		uint32_t IndexCount;
	};
#pragma pack(pop)

	class ModelSerializer
	{
	public:
		static void Serialize(std::vector<MeshHandle> meshHandles, const std::string& path, const std::string& toReplace = "") noexcept;
		static void SerializeBinary(MeshHandle& meshHandle, const std::string& path) noexcept;
		static MeshHandle Deserialize(const std::string& fullPath, const std::shared_ptr<Scene> pScene = nullptr) noexcept;
	};
}