#include "IFactory.h"
#include "../../Subsystem/AssetImportSubsystem.h"
#include "../../Core/Editor.h"
namespace Relentless
{
	const FactoryResult& IFactory::ImportFromFile(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext) noexcept
	{
		const FactoryResult& result = ImportFromFileImpl(aPath, aPackagePath, aName, aFeedbackContext);
		
		std::vector<FactoryResult> allResults;
		allResults.reserve(m_AdditionalImportedAssets.size() + 1);

		allResults.push_back(result);
		allResults.insert(allResults.end(), m_AdditionalImportedAssets.begin(), m_AdditionalImportedAssets.end());

		Ref<IFactory> pFactory = this;
		Application::Get().SubmitToMainThread([pFactory, results = std::move(allResults)]()
			{
				AssetImportSubsystem* pImportSubsystem = Editor::Get()->GetSubsystem<AssetImportSubsystem>();
				if (!pImportSubsystem)
					return;

				for (const auto& result : results)
				{
					if (result)
						pImportSubsystem->OnPostAssetImported(pFactory, result.value());
				}
			});

		return result;
	}
}
