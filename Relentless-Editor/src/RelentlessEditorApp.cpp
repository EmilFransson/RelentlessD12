#include "EditorLayer.h"
#include "Relentless/EntryPoint.h"

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 602; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = "d3d12\\"; }

namespace Relentless
{
	class RelentlessEditor : public Application 
	{
	public:
		RelentlessEditor(const ApplicationSpecification& applicationSpecification) noexcept
			: Application{applicationSpecification}
		{
			PushLayer(std::make_unique<EditorLayer>());
		}
		virtual ~RelentlessEditor() noexcept override final = default;
	};

	const std::unique_ptr<Application> CreateApplication() noexcept
	{
		ApplicationSpecification applicationSpecification;
		applicationSpecification.Name = std::string("Relentless-Editor") + std::string(APPLICATION_SUFFIX);

		return std::unique_ptr<RelentlessEditor>(RLS_NEW RelentlessEditor(applicationSpecification));
	}
}