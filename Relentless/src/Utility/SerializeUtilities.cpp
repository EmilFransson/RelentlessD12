#include "SerializeUtilities.h"

namespace Relentless
{
	YAML::Emitter& operator<<(YAML::Emitter& out, DirectX::XMFLOAT4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, DirectX::XMFLOAT3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, DirectX::XMFLOAT2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, UUID& id)
	{
		wchar_t wszGuid[40] = { 0 };
		StringFromGUID2(id, wszGuid, 39);
		std::wstring ws = std::wstring(wszGuid);
		std::string s = ConvertWideStringToString(ws);
		out << s;
		return out;
	}

	UUID ConvertStringToGUID(const std::string& guidString) noexcept
	{
		std::wstring GUIDAsWString = ConvertStringToWstring(guidString);
		GUID uuid;
		IIDFromString(GUIDAsWString.c_str(), &uuid);
		return uuid;
	}
}