#pragma once
#include "ITreeNode.h"

#include "UI/IWidget.h"

namespace Relentless
{
	class IDetailsTreeNode : public ITreeNode
	{
	public:
		IDetailsTreeNode() noexcept = default;
		virtual ~IDetailsTreeNode() noexcept = default;
	};
}