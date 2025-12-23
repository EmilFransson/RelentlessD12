#pragma once

#include <Relentless.h>
#include "IEditorPanel.h"

namespace Relentless
{
	enum class ESelectionState : uint8;
	enum class EAxis : uint8 { X, Y, Z };

	class EntityDetailsView;

	class DetailsPanel : public IEditorPanel
	{
	public:
		DetailsPanel(std::weak_ptr<Editor> aEditor) noexcept;
		virtual ~DetailsPanel() noexcept override = default;
	private:
		template<typename Widget>
		Widget* AddRow(
			Table* table,
			uint32_t    row,
			char const* labelText,
			Widget* widget)
		{
			table->Add(new Label(labelText), 0, row);
			table->Add(widget, 1, row);
			return widget;
		}

		//[[nodiscard]] Ref<IBaseWidget> CreateBaseSection() noexcept;

		//template<typename ComponentType>
		//[[nodiscard]] Ref<IBaseWidget> CreateComponentSection() noexcept;

		//template<typename ComponentType>
		//void ConditionallyCreateSection(Ref<VerticalBox> pRoot, EntityManager& entityManager) noexcept;
		
		//void CreateEmpty() noexcept;
		//void CreateFromSelection() noexcept;

		//[[nodiscard]] Vector3 GetLocation(ComboBox* pTransformSpaceComboBox) const noexcept;
		//[[nodiscard]] Vector3 GetRotation(ComboBox* pTransformSpaceComboBox) const noexcept;
		//[[nodiscard]] Vector3 GetScale(ComboBox* pTransformSpaceComboBox) const noexcept;

		//void OnLightTypeSelectionChanged(const char* selected) noexcept;

		//----Directional Lights----:
		//void OnDirectionalLightColorChanged(const Color& color) noexcept;
		//void OnDirectionalLightIntensityChanged(float intensity) noexcept;
		//void OnDirectionalLightTemperatureChanged(float temperature) noexcept;
		//void OnDirectionalLightUseTemperatureChanged(bool useTemperature) noexcept;
		//[[nodiscard]] Color OnDirectionalLightColorRequested() const noexcept;
		//[[nodiscard]] float OnDirectionalLightIntensityRequested() const noexcept;
		//[[nodiscard]] float OnDirectionalLightTemperatureRequested() const noexcept;
		//[[nodiscard]] bool OnDirectionalLightUseTemperatureRequested() const noexcept;
		//
		////----Point Lights----
		//void OnPointLightAttenuationRadiusChanged(float radius) noexcept;
		//void OnPointLightColorChanged(const Color& color) noexcept;
		//void OnPointLightIntensityChanged(float intensity, const char* pIntensityUnit) noexcept;
		//void OnPointLightTemperatureChanged(float temperature) noexcept;
		//void OnPointLightUseTemperatureChanged(bool useTemperature) noexcept;
		//[[nodiscard]] float OnPointLightAttenuationRadiusRequested() const noexcept;
		//[[nodiscard]] Color OnPointLightColorRequested() const noexcept;
		//[[nodiscard]] float OnPointLightIntensityRequested(const char* pIntensityUnit) const noexcept;
		//[[nodiscard]] float OnPointLightTemperatureRequested() const noexcept;
		//[[nodiscard]] bool OnPointLightUseTemperatureRequested() const noexcept;

		//----Spot Lights----
		//void OnSpotLightAttenuationRadiusChanged(float radius) noexcept;
		//void OnSpotLightColorChanged(const Color& color) noexcept;
		//void OnSpotLightInnerConeAngleChanged(float angleDeg) noexcept;
		//void OnSpotLightIntensityChanged(float intensity, const char* pIntensityUnit) noexcept;
		//void OnSpotLightOuterConeAngleChanged(float angleDeg) noexcept;
		//void OnSpotLightTemperatureChanged(float temperature) noexcept;
		//void OnSpotLightUseTemperatureChanged(bool useTemperature) noexcept;
		//[[nodiscard]] float OnSpotLightAttenuationRadiusRequested() const noexcept;
		//[[nodiscard]] Color OnSpotLightColorRequested() const noexcept;
		//[[nodiscard]] float OnSpotLightInnerConeAngleRequested() const noexcept;
		//[[nodiscard]] float OnSpotLightIntensityRequested(const char* pIntensityUnit) const noexcept;
		//[[nodiscard]] float OnSpotLightOuterConeAngleRequested() const noexcept;
		//[[nodiscard]] float OnSpotLightTemperatureRequested() const noexcept;
		//[[nodiscard]] bool OnSpotLightUseTemperatureRequested() const noexcept;

		//void OnLocationChanged(float value, EAxis axis, ETransformSpace space) noexcept;
		//void OnLocationChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept;
		//void OnRotationChanged(float value, EAxis axis, ETransformSpace space) noexcept;
		//void OnRotationChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept;
		//void OnScaleChanged(float value, EAxis axis, ETransformSpace space) noexcept;
		//void OnScaleChanged(const Vector3& value, ComboBox* pTransformSpaceComboBox) noexcept;

		void OnEntityDestroyed(entity destroyedEntity) noexcept;

		void OnPreSceneChanged(Scene* pScene) noexcept;

		//void OnRender() noexcept override;

		void OnSceneChanged(Scene* pScene) noexcept;
		void OnSelectionChanged(entity e, ESelectionState selectionState) noexcept;
	private:
		std::vector<entity> m_SelectedEntities;
		std::vector<Ref<IDetailsTreeNode>> m_Nodes;

		bool m_SelectionLocked = false;

		Ref<EntityDetailsView> m_pEntityDetailsView = nullptr;
	};

	//template<> Ref<IBaseWidget> DetailsPanel::CreateComponentSection<DirectionalLightComponent>() noexcept;
	//template<> Ref<IBaseWidget> DetailsPanel::CreateComponentSection<PointLightComponent>() noexcept;
	//template<> Ref<IBaseWidget> DetailsPanel::CreateComponentSection<SpotLightComponent>() noexcept;
	//template<> Ref<IBaseWidget> DetailsPanel::CreateComponentSection<TransformComponent>() noexcept;
	//
	//template<typename ComponentType>
	//void DetailsPanel::ConditionallyCreateSection(Ref<VerticalBox> pRoot, EntityManager& entityManager) noexcept
	//{
	//	if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager, &pRoot](entity e)
	//		{
	//			return entityManager.Has<ComponentType>(e);
	//		}))
	//	{
	//		pRoot->Add(CreateComponentSection<ComponentType>());
	//	}
	//}
}