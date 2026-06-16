#pragma once
#include "DragDropOperation.h"

namespace Relentless
{
	class Label;

	class PathDragDropOperation : public DragDropOperation<PathDragDropOperation>
	{
	public:
		explicit PathDragDropOperation(const std::vector<String>& somePaths, const String& aPreviewText) noexcept;

		virtual void CreatePreview() noexcept override;

		NO_DISCARD uint32 GetNumDraggedPaths() const noexcept;
		NO_DISCARD const std::vector<String>& GetPaths() const noexcept;
		NO_DISCARD const String& GetPrimaryDraggedPath() const noexcept;

		void SetDrawSymbolLabel(bool aState) noexcept;
		void SetPreviewText(const String& aPreviewText) noexcept;
		void SetSymbol(const String& aSymbolLabel, const Color& aColor) noexcept;
	private:
		std::vector<String> m_Paths;
		String m_PreviewText;
		
		Label* m_pPreviewLabel = nullptr;
		Label* m_pSymbolLabel = nullptr;
	};
}