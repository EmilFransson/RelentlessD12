#pragma once
#include <Relentless.h>

namespace Relentless
{
	class FactoryBase : public IFactory
	{
	public:
		virtual FactoryResult ImportFromFile(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept override final;
	protected:
		virtual FactoryResult ImportFromFileImpl(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept override;
	};
}