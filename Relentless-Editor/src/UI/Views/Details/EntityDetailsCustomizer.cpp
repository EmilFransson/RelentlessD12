#include "EntityDetailsCustomizer.h"

#include "../../../Core/Editor.h"

namespace Relentless
{
	using namespace std::placeholders;

	EntityDetailsCustomizer::EntityDetailsCustomizer(Editor* pEditor) noexcept
		: IDetailsCustomizer(pEditor)
	{
	}

	std::vector<Ref<IDetailsTreeNode>> EntityDetailsCustomizer::Build() noexcept
	{
		std::vector<Ref<IDetailsTreeNode>> nodes;
		if (m_Entities.empty())
			return nodes;

		nodes.push_back(CreateTransformSection());

		return nodes;
	}

	Vector3 EntityDetailsCustomizer::GetLocation(ComboBox* pTransformSpaceComboBox) const noexcept
	{
		const ETransformSpace transformSpace = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());

		switch (transformSpace)
		{
		case ETransformSpace::Relative:
			return m_pEditor->GetActiveScene()->GetLocalLocation(m_Entities[0]);
		case ETransformSpace::Absolute:
			return m_pEditor->GetActiveScene()->GetWorldLocation(m_Entities[0]);
		default:
			RLS_ASSERT(false, "Unreachable.");
			return Vector3::Zero;
		}
	}

	void EntityDetailsCustomizer::OnLocationChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept
	{
		Scene* pScene = m_pEditor->GetActiveScene();
		const ETransformSpace space = static_cast<ETransformSpace>(pTransformSpaceComboBox->GetSelectedIndex());

		std::for_each(m_Entities.begin(), m_Entities.end(), [&](entity e)
			{
				switch (space)
				{
				case ETransformSpace::Absolute: pScene->SetWorldLocation(e, value); break;
				case ETransformSpace::Relative: pScene->SetLocalLocation(e, value); break;
				}
			});
	}

	void EntityDetailsCustomizer::SetEntities(const std::vector<entity>& entities) noexcept
	{
		m_Entities = entities;
	}

	Ref<IDetailsTreeNode> EntityDetailsCustomizer::CreateTransformSection() noexcept
	{
		Ref<DetailCategoryNode> pTransformCategoryNode = new DetailCategoryNode();
		pTransformCategoryNode->SetWidgetCallback([](ITreeNode* pNode)
			{
				Ref<DetailCategoryRow> pDetailCategory = new DetailCategoryRow();
				return pDetailCategory;
			});

		DetailRowNode* pNode = pTransformCategoryNode->AddChild();
		pNode->SetWidgetCallback([this](ITreeNode* pNode)
			{
				Ref<DetailPropertyRow> pRow = new DetailPropertyRow();

				ComboBox* pLocationComboBox = pRow->SetNameContent(new ComboBox())
					->AddSelectables({ "Location", "Absolute Location" });

				pRow->SetValueContent(new Float3Drag(0.01f, -FLT_MAX, FLT_MAX, "%.2f"))
					->Value(std::bind(&EntityDetailsCustomizer::GetLocation, this, pLocationComboBox))
					->OnValueChanged(std::bind(&EntityDetailsCustomizer::OnLocationChanged, this, _1, pLocationComboBox))
					->SetIndicatorColor(0, Colors::OffRed)
					->SetIndicatorColor(1, Colors::OffGreen)
					->SetIndicatorColor(2, Colors::OffBlue)
					->SetSizePolicy(ESizePolicy::Stretch);

				return pRow;
			});

		return pTransformCategoryNode;
	}
}