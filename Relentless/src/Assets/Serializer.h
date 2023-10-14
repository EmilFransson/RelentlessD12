#pragma once
#include "../Graphics/Resources/Texture.h"

namespace Relentless
{
	class Serializer
	{
	public:
	private:
		template<typename>
		struct always_false : std::false_type {};

	public:
		template<typename AssetType>
		static [[nodiscard]] AssetType Deserialize(const std::string& filename) noexcept
		{
			static_assert(always_false<AssetType>::value, "This operation is not supported by the type, or the type does not exist.");
		}
	private:

	};

	template<>
	inline Texture2D Serializer::Deserialize<Texture2D>(const std::string& filename) noexcept
	{
		Texture2D tex{};
		return std::move(tex);
	}
}