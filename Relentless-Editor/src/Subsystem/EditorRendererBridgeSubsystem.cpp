#include "EditorRendererBridgeSubsystem.h"

#include "Assets/Factory/TextureFactory.h"

#include "Core/RelentlessEditorApp.h"

namespace Relentless
{
	entity EditorRendererBridgeSubsystem::GetHoveredEntity() const noexcept
	{
		return m_HoveredEntity;
	}

	void EditorRendererBridgeSubsystem::OnEntityReadbackDone(uint32 aEntityID) noexcept
	{
		/*
		 An id of 0 is considered a sentinel value for the read back results.
		 This means all actual entity ids are shifted up by one and should be downshifted again (if entityID != 0)
		*/

		Scene* pScene = m_pEditor->GetActiveScene();
		if (!pScene)
			return;

		if (aEntityID == 0u)
			m_HoveredEntity = NULL_ENTITY;
		else
		{
			const uint32 actualEntityID = aEntityID - 1;
			m_HoveredEntity = pScene->GetEntityManager().GetEntityFromIdentity(actualEntityID);
		}
	}

	bool EditorRendererBridgeSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pEditor = static_cast<Editor*>(aSystemManager);

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());

		Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
		pFactory->SetImportAsSRGB(false);
		pFactory->SetGenerateMipmaps(false);

		std::vector<AssetImportTask> tasks;
		AssetImportTask& task = tasks.emplace_back();
		task.FilePath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Textures/brdf_ibl_lut.dds");
		task.pFactory = pFactory;
		task.DestinationPath = "Engine/Textures/";

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		const std::vector<AssetImportResult> results = assetToolsModule.Import(tasks);
		RLS_VERIFY(!results.empty(), "[EditorRendererBridgeSubsystem::OnLoad]: Failed to import BRDF LUT.");

		m_BRDFLutTextureHandle = results.front().Handle;
		
		const UniquePtr<Renderer>& pRenderer = app.GetRenderer();
		pRenderer->OnEntityIDReadbackDone.Connect(this, &EditorRendererBridgeSubsystem::OnEntityReadbackDone);
		pRenderer->OnRequestBRDFLut(this, &EditorRendererBridgeSubsystem::OnRequestBRDFLut);

		return true;
	}

	AssetHandle EditorRendererBridgeSubsystem::OnRequestBRDFLut() noexcept
	{
		return m_BRDFLutTextureHandle;
	}

	bool EditorRendererBridgeSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

}