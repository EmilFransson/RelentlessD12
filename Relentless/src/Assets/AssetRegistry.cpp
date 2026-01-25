// #include "AssetRegistry.h"
// #include "AssetMeta.h"
// #include "Assets/Serializer.h"
// #include "File/File.h"
// #include "Utility/Common.h"
// #include "Utility/FilepathUtils.h"
// 
// namespace Relentless
// {
// 	std::mutex AssetRegistry::m_Mutex{};
// 
// 	struct BiDirectionalMap
// 	{
// 		//std::unordered_map<UUID, std::filesystem::path> UUIDToPathMap;
// 		//std::unordered_map<std::filesystem::path, AssetMetaData> PathToAssetMetaMap;
// 	};
// 	static BiDirectionalMap s_Data;
// 
// 	void AssetRegistry::MapAssetToFilepath(const AssetMetaData& metaData, const std::string& filepath) noexcept
// 	{
// 		//RLS_ASSERT(!IsUUIDMapped(metaData.Uuid), "UUID is already mapped.");
// 		//RLS_ASSERT(!IsFilepathMapped(filepath), "Filepath is already mapped.");
// 		//RLS_ASSERT(IsFilepathValid(filepath), "Filepath is not valid.");
// 
// 		std::lock_guard<std::mutex> guard(m_Mutex);
// 
// 		//s_Data.UUIDToPathMap[metaData.Uuid] = filepath;
// 		//s_Data.PathToAssetMetaMap[filepath] = metaData;
// 		//
// 		//RLS_CORE_INFO("Mapped Asset");
// 		//RLS_CORE_INFO("Path: {0}", filepath);
// 		//RLS_CORE_INFO("Type: {0}", std::to_string((int)metaData.AssetType));
// 		//RLS_CORE_INFO("UUID: {0}", ConvertUUIDToString(metaData.Uuid));
// 	}
// 	
// 	bool AssetRegistry::IsUUIDMapped(const UUID& uuid) noexcept
// 	{
// 		return false;
// 		//std::lock_guard<std::mutex> guard(m_Mutex);
// 		//return s_Data.UUIDToPathMap.contains(uuid);
// 	}
// 	
// 	bool AssetRegistry::IsFilepathMapped(const std::string& filepath) noexcept
// 	{
// 		return false;
// 
// 		//std::lock_guard<std::mutex> guard(m_Mutex);
// 		//return s_Data.PathToAssetMetaMap.contains(filepath);
// 	}
// 	
// 	bool AssetRegistry::IsUUIDToFilepathMapped(const UUID& uuid, const std::string& filepath) noexcept
// 	{
// 		std::lock_guard<std::mutex> guard(m_Mutex);
// 
// 		//return s_Data.UUIDToPathMap.contains(uuid) 
// 		//	&& s_Data.UUIDToPathMap[uuid] == filepath 
// 		//	&& s_Data.PathToAssetMetaMap.contains(filepath)
// 		//	&& s_Data.PathToAssetMetaMap[filepath].Uuid == uuid;
// 		return false;
// 	}
// 	
// 	void AssetRegistry::RemoveUUIDToFilepathMap(const UUID& uuid, const std::string& filepath) noexcept
// 	{
// 		RLS_ASSERT(IsUUIDToFilepathMapped(uuid, filepath), "UUID and filepath is not properly mapped..");
// 
// 		std::lock_guard<std::mutex> guard(m_Mutex);
// 		
// 		//s_Data.UUIDToPathMap.erase(uuid);
// 		//s_Data.PathToAssetMetaMap.erase(filepath);
// 	}
// 	
// 	void AssetRegistry::Reset() noexcept
// 	{
// 		std::lock_guard<std::mutex> guard(m_Mutex);
// 
// 		//s_Data.PathToAssetMetaMap.clear();
// 		//s_Data.UUIDToPathMap.clear();
// 	}
// 	
// 	void AssetRegistry::ChangeMappedFilepath(const UUID& uuid, const std::string&) noexcept
// 	{
// 		//RLS_ASSERT(IsUUIDMapped(uuid), "UUID is not mapped to any filepath");
// 		//const std::filesystem::path currentFilepath = s_Data.UUIDToPathMap[uuid];
// 		//const AssetMetaData data = s_Data.PathToAssetMetaMap[currentFilepath];
// 		//
// 		//{
// 		//	std::lock_guard<std::mutex> guard(m_Mutex);
// 		//	
// 		//	s_Data.UUIDToPathMap.erase(uuid);
// 		//	s_Data.PathToAssetMetaMap.erase(currentFilepath);
// 		//}
// 		//RLS_ASSERT(false, "TODO");
// 		//MapAssetToFilepath({data.Uuid, data.AssetType}, newfilepath);
// 	}
// 	
// 	//Checking one is sufficient as it acts as a bidirectional map
// 	uint32_t AssetRegistry::GetMappedAssetCount() noexcept
// 	{
// 		std::lock_guard<std::mutex> guard(m_Mutex);
// 		
// 		return 0u;
// 		//return static_cast<uint32_t>(s_Data.UUIDToPathMap.size());
// 	}
// 	
// 	const std::filesystem::path& AssetRegistry::GetFilepath(const UUID& uuid) noexcept
// 	{
// 		RLS_ASSERT(IsUUIDMapped(uuid), "UUID is not mapped to any filepath.");
// 		std::lock_guard<std::mutex> guard(m_Mutex);
// 		return "";
// 		//return s_Data.UUIDToPathMap[uuid];
// 	}
// 	
// 	const UUID& AssetRegistry::GetUUID(const std::string& filepath) noexcept
// 	{
// 		RLS_ASSERT(IsFilepathMapped(filepath), "filepath is not mapped to any UUID.");
// 	
// 		//std::lock_guard<std::mutex> guard(m_Mutex);
// 		//return s_Data.PathToAssetMetaMap[filepath].Uuid;
// 		return { 0 };
// 	}
// 	
// 	void AssetRegistry::RecursiveScanDirectoryForAssets(const std::filesystem::path& startingDirectory) noexcept
// 	{
// 		RLS_ASSERT(File::ExistsDir(startingDirectory), "Path is invalid.");
// 		
// 		for (const auto& entry : std::filesystem::recursive_directory_iterator(startingDirectory))
// 		{
// 			if (!std::filesystem::is_regular_file(entry.path()))
// 				continue;
// 		
// 			const std::string extension = FilepathUtils::ExtractExtension(entry);
// 			if (extension != ASSET_EXTENSION)
// 				continue;
// 			
// 			std::ifstream inFile(entry.path().string(), std::ios_base::binary);
// 			RLS_ASSERT(inFile.is_open(), "Unable to open rasset file.");
// 			
// 			auto [signature, version] = Serializer::DeserializeSignatureAndVersion(inFile);
// 
// 			RassetHeader_1 readHeader = Serializer::DeserializeRAssetHeaderVersion1(inFile);
// 
// 			//AssetMetaData metaData{};
// 			//metaData.Name = std::string(readHeader.Name);
// 			//metaData.AssetType = readHeader.AssetType;
// 			//metaData.Uuid = readHeader.UUID;
// 			//metaData.SourcePath = std::string(readHeader.SourcePath);
// 			//metaData.ModificationDateAndTime = readHeader.ModificationDateAndTime;
// 			//metaData.AssetFlags = readHeader.AssetFlags;
// 			//
// 			//if (readHeader.TagsByteSize > 0)
// 			//	metaData.Tags = Serializer::DeserializeAssetTags(inFile, readHeader.TagsByteSize);
// 			
// 			//Map(entry.path(), metaData, MapOperation::Override);
// 			//inFile.close();
// 		}
// 	}
// 
// 	//const AssetMetaData& AssetRegistry::GetMetaData(const std::filesystem::path& filepath) noexcept
// 	//{
// 	//	RLS_ASSERT(std::filesystem::exists(filepath), "[AssetRegistry]: Filepath is invalid.");
// 	//	RLS_ASSERT(IsFilepathMapped(filepath.string()), "[AssetRegistry]: Filepath is not mapped.");
// 	//	
// 	//	std::lock_guard<std::mutex> guard(m_Mutex);
// 	//	return {};
// 	//	//return s_Data.PathToAssetMetaMap[filepath.string()];
// 	//}
// 
// 	//const AssetMetaData& AssetRegistry::GetMetaData(const AssetHandle& handle) noexcept
// 	//{
// 	//	std::lock_guard<std::mutex> guard(m_Mutex);
// 	//	return {};
// 	//}
// 
// 	void AssetRegistry::Map(const std::filesystem::path& path, const AssetMetaData& metaData, MapOperation operation) noexcept
// 	{
// 		//std::lock_guard<std::mutex> guard(m_Mutex);
// 		//if (operation == MapOperation::Override)
// 		//{
// 		//	s_Data.UUIDToPathMap.insert_or_assign(metaData.Uuid, path);
// 		//	s_Data.PathToAssetMetaMap.insert_or_assign(path, metaData);
// 		//}
// 		//else
// 		//{
// 		//	if (!s_Data.UUIDToPathMap.contains(metaData.Uuid))
// 		//		s_Data.UUIDToPathMap[metaData.Uuid] = path;
// 		//	if (!s_Data.PathToAssetMetaMap.contains(path))
// 		//		s_Data.PathToAssetMetaMap[path] = metaData;
// 		//}
// 	}
// }