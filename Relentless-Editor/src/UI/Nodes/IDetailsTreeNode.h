#pragma once
#include "ITreeNode.h"

namespace Relentless
{
	class IDetailsTreeNode : public ITreeNode
	{
	public:
		IDetailsTreeNode() noexcept = default;
		virtual ~IDetailsTreeNode() noexcept = default;
	};
}