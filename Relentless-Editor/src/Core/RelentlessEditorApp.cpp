#include "EditorLayer.h"
#include "EntryPoint.h"
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 606; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = "D3D12\\"; }
namespace Relentless
{
	class RelentlessEditor : public Application 
	{
	public:
		RelentlessEditor(const ApplicationSpecification& applicationSpecification) noexcept
			: Application{applicationSpecification}
		{
			PushLayer(&m_EditorLayer);
		}
		virtual ~RelentlessEditor() noexcept override final = default;
	private:
		EditorLayer m_EditorLayer;
	};

	const std::unique_ptr<Application> CreateApplication() noexcept
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.Name = std::string("Relentless-Editor") + std::string(APPLICATION_SUFFIX);

		return std::unique_ptr<RelentlessEditor>(RLS_NEW RelentlessEditor(applicationSpecification));
	}
}