#pragma once
#include "Panel.h"

namespace Relentless
{
	class Editor;
	enum class ESelectionState : uint8;

	enum class ETransformSpace : int { Relative = 0, Absolute };
	enum class EAxis : uint8 { X, Y, Z };

	class DetailsPanel : public PanelBase
	{
	public:
		DetailsPanel(const char* pName, ImGuiWindowFlags flags, Editor* pEditor) noexcept;
		virtual ~DetailsPanel() noexcept override;
	protected:
		virtual void OnRender() noexcept override {}
	private:
		template<typename ComponentType>
		[[nodiscard]] Ref<IWidget> CreateComponentSection(const std::vector<entity>& selectedEntities) noexcept;

		template<typename ComponentType>
		void ConditionallyCreateSection(Ref<VerticalBox> pRoot, EntityManager& entityManager, const std::vector<entity>& selectedEntities) noexcept;
		
		[[nodiscard]] Vector3 GetLocation(ComboBox* pTransformSpaceComboBox) const noexcept;
		[[nodiscard]] Vector3 GetRotation(ComboBox* pTransformSpaceComboBox) const noexcept;
		[[nodiscard]] Vector3 GetScale(ComboBox* pTransformSpaceComboBox) const noexcept;

		void OnLocationChanged(float value, EAxis axis, ETransformSpace space) noexcept;
		void OnRotationChanged(float value, EAxis axis, ETransformSpace space) noexcept;
		void OnScaleChanged(float value, EAxis axis, ETransformSpace space) noexcept;

		void OnSelectionChanged(entity e, ESelectionState selectionState) noexcept;
	private:
		Editor* m_pEditor = nullptr;
	};

	template<> Ref<IWidget> DetailsPanel::CreateComponentSection<DirectionalLightComponent>(const std::vector<entity>& selectedEntities) noexcept;
	template<> Ref<IWidget> DetailsPanel::CreateComponentSection<TransformComponent>(const std::vector<entity>& selectedEntities) noexcept;

	template<typename ComponentType>
	void DetailsPanel::ConditionallyCreateSection(Ref<VerticalBox> pRoot, EntityManager& entityManager, const std::vector<entity>& selectedEntities) noexcept
	{
		if (std::all_of(selectedEntities.begin(), selectedEntities.end(), [&entityManager, &pRoot](entity e)
			{
				return entityManager.Has<ComponentType>(e);
			}))
		{
			pRoot->Add(CreateComponentSection<ComponentType>(selectedEntities));
		}
	}

}