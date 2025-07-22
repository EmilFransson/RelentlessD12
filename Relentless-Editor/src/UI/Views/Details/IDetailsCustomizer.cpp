#include "IDetailsCustomizer.h"

#include "../../../Core/Editor.h"

namespace Relentless
{
	IDetailsCustomizer::IDetailsCustomizer(Editor* pEditor) noexcept
		: m_pEditor(pEditor)
	{
	}
}