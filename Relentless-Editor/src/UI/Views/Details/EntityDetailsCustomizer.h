#pragma once
#include <Relentless.h>

#include "IDetailsCustomizer.h"

namespace Relentless
{
	enum class ETransformSpace : int { Relative = 0, Absolute };
	//enum class EAxis : uint8 { X, Y, Z };

	class EntityDetailsCustomizer : public IDetailsCustomizer
	{
	public:
		EntityDetailsCustomizer(Editor* pEditor) noexcept;

		//Already fail: this means it cant inspect anything else and demands entities and manager etc. Rethink
		virtual [[nodiscard]] std::vector<Ref<IDetailsTreeNode>> Build() noexcept override;
		
		[[nodiscard]] Vector3 GetLocation(ComboBox* pTransformSpaceComboBox) const noexcept;
		
		void OnLocationChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept;

		void SetEntities(const std::vector<entity>& entities) noexcept;
	private:
		NO_DISCARD Ref<IDetailsTreeNode> CreateTransformSection() noexcept;
	private:
		std::vector<entity> m_Entities;
	};
}