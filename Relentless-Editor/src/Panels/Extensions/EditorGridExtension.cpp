#include "EditorGridExtension.h"

#include <Relentless.h>

#include "Panels/ViewportPanel.h"

#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	void EditorGridExtension::CustomizeGridDetails(IDetailLayoutBuilder& aBuilder)
	{
		IDetailCategoryBuilder& gridCategoryBuilder = aBuilder.EditCategory(ICON_FA_TABLE_CELLS "  Grid");

		gridCategoryBuilder.AddProperty<bool>("Enabled",
			[this]() { return m_pViewportClient->GetRenderFeatures().IsEnabled(ERenderFeature::Grid); },
			[this](const bool& aEnabled) { m_pViewportClient->SetRenderFeature(ERenderFeature::Grid, aEnabled); },
			true)
			.NameSlot().Label("Enabled")
			.ValueSlot().CheckBox();

		gridCategoryBuilder.AddProperty<Color>("Major Color",
			[this]() { return m_pEditorGridPass->GetMajorColor(); },
			[this](const Color& aColor) { m_pEditorGridPass->SetMajorColor(aColor); },
			EditorGrid::DEFAULT_MAJOR_COLOR)
			.NameSlot().Label("Color Major")
			.ValueSlot().ColorPicker();

		gridCategoryBuilder.AddProperty<Color>("Minor Color",
			[this]() { return m_pEditorGridPass->GetMinorColor(); },
			[this](const Color& aColor) { m_pEditorGridPass->SetMinorColor(aColor); },
			EditorGrid::DEFAULT_MINOR_COLOR)
			.NameSlot().Label("Color Minor")
			.ValueSlot().ColorPicker();

		gridCategoryBuilder.AddProperty<float>("Height Offset",
			[this]() { return m_pEditorGridPass->GetHeightOffset(); },
			[this](const float& aHeight) { m_pEditorGridPass->SetHeightOffset(aHeight); },
			0.0f)
			.NameSlot().Label("Height Offset")
			.ValueSlot().SpinBox().Range(-FLT_MAX, FLT_MAX);

		gridCategoryBuilder.AddProperty<Vector3>("Rotation",
			[this]() 
			{ 
				const Vector3& rotationRadians = m_pEditorGridPass->GetRotationRadians(); 
				return Vector3(Math::RadToDeg360(rotationRadians.x), Math::RadToDeg360(rotationRadians.y), Math::RadToDeg360(rotationRadians.z));
			},
			[this](const Vector3& aRotationEuler) 
			{ 
				m_pEditorGridPass->SetRotationRadians(Vector3(Math::DegToRad(aRotationEuler.x), Math::DegToRad(aRotationEuler.y), Math::DegToRad(aRotationEuler.z)));
			},
			Vector3::Zero)
			.NameSlot().Label("Rotation")
			.ValueSlot().SpinBox().Range(-FLT_MAX, FLT_MAX);

		gridCategoryBuilder.AddProperty<float>("Spacing",
			[this]() { return m_pEditorGridPass->GetSpacing(); },
			[this](const float& aSpacing) { m_pEditorGridPass->SetSpacing(aSpacing); },
			1.0f)
			.NameSlot().Label("Spacing")
			.ValueSlot().SpinBox().Range(0.5f, 100.0f).Delta(0.1f);

		gridCategoryBuilder.AddProperty<float>("Distance Fade",
			[this]() { return m_pEditorGridPass->GetDistanceFade(); },
			[this](const float& aFade) { m_pEditorGridPass->SetDistanceFade(aFade); },
			200.0f)
			.NameSlot().Label("Distance Fade")
			.ValueSlot().SpinBox().Range(0.0f, 400.0f);

		gridCategoryBuilder.AddProperty<float>("Height Fade",
			[this]() { return m_pEditorGridPass->GetHeightFade(); },
			[this](const float& aFade) { m_pEditorGridPass->SetHeightFade(aFade); },
			150.0f)
			.NameSlot().Label("Height Fade")
			.ValueSlot().SpinBox().Range(0.0f, FLT_MAX);

		gridCategoryBuilder.AddProperty<float>("Max Opacity",
			[this]() { return m_pEditorGridPass->GetMaxOpacity(); },
			[this](const float& aOpacity) { m_pEditorGridPass->SetMaxOpacity(aOpacity); },
			1.0f)
			.NameSlot().Label("Max Opacity")
			.ValueSlot().Slider().Range(0.0f, 1.0f);
	}

	void EditorGridExtension::OnRegistered(ViewportPanel& aViewportPanel)
	{
		m_pViewportClient = &aViewportPanel.GetClient();

		m_pEditorGridPass = MakeUnique<EditorGrid>(Application::Get().GetGraphicsDevice());
		Renderer::RegisterRenderCallback(ERenderPhase::PostTonemap, Callback<void(CommandContext&, const RenderView&, SceneTextures&)>::Bind(this, &EditorGridExtension::OnRender));
	}

	void EditorGridExtension::OnRender(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures)
	{
		if (m_pViewportClient->GetRenderFeatures().IsEnabled(ERenderFeature::Grid))
			m_pEditorGridPass->Render(aCommandContext, aRenderView, aSceneTextures);
	}

	void EditorGridExtension::OnUnregister()
	{
		throw std::logic_error("The method or operation is not implemented.");
	}
}