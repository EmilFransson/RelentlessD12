#pragma once
#include "Utility/Common.h"

namespace Relentless
{
	class IAsset : public RefCounted<IAsset>
	{
	public:
		explicit IAsset(const UUID& aUUID = CreateUUID()) noexcept;
		virtual ~IAsset() = default;
		NO_DISCARD const String& GetName() const noexcept;
		NO_DISCARD const UUID& GetUUID() const noexcept;

		void SetName(const String& aName) noexcept;
	private:
		String m_Name;
		UUID m_UUID;
	};
}