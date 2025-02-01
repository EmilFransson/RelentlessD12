#pragma once
#include "RHI.h"

namespace Relentless
{
	#define VERIFY_HR(hr) D3D::LogHRESULT(hr, nullptr, #hr, __FILE__, __LINE__)
	#define VERIFY_HR_EX(hr, device) D3D::LogHRESULT(hr, device, #hr, __FILE__, __LINE__)

	namespace D3D
	{
		inline [[nodiscard]] std::string CommandListTypeToString(D3D12_COMMAND_LIST_TYPE type) noexcept
		{
			switch (type)
			{
			case D3D12_COMMAND_LIST_TYPE_BUNDLE:
				return "D3D12_COMMAND_LIST_TYPE_BUNDLE";
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return "D3D12_COMMAND_LIST_TYPE_COMPUTE";
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return "D3D12_COMMAND_LIST_TYPE_COPY";
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return "D3D12_COMMAND_LIST_TYPE_DIRECT";
			case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
				return "D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE";
			case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
				return "D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE";
			case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
				return "D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS";
			default:
				RLS_ASSERT(false, "Unreachable");
				return {};
			}
		}

		inline std::string GetErrorString(HRESULT errorCode, ID3D12Device* pDevice)
		{
			std::string str;
			char* errorMsg;
			if (::FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&errorMsg, 0, nullptr) != 0)
			{
				str += errorMsg;
				::LocalFree(errorMsg);
			}
			if (errorCode == DXGI_ERROR_DEVICE_REMOVED && pDevice)
			{
				Ref<ID3D12InfoQueue> pInfo;
				pDevice->QueryInterface(pInfo.GetAddressOf());
				if (pInfo)
				{
					str += "Validation Layer: \n";
					for (uint64_t i = 0; i < pInfo->GetNumStoredMessages(); ++i)
					{
						size_t messageLength = 0;
						pInfo->GetMessageA(0, nullptr, &messageLength);
						D3D12_MESSAGE* pMessage = (D3D12_MESSAGE*)malloc(messageLength);
						pInfo->GetMessageA(0, pMessage, &messageLength);
						str += pMessage->pDescription;
						str += "\n";
						free(pMessage);
					}
				}

				HRESULT removedReason = pDevice->GetDeviceRemovedReason();
				str += "\nDRED: " + GetErrorString(removedReason, nullptr);
			}
			return str;
		}

		inline bool LogHRESULT(HRESULT hr, ID3D12Device* pDevice, const char* pCode, const char* pFileName, uint32_t lineNumber)
		{
			if (!SUCCEEDED(hr))
			{
				RLS_CORE_ERROR("{0}:{1}: {2} - {3}", pFileName, lineNumber, GetErrorString(hr, pDevice).c_str(), pCode);
				__debugbreak();
				return false;
			}
			return true;
		}

		inline static bool HasWriteResourceState(D3D12_RESOURCE_STATES state)
		{
			return EnumHasAnyFlags(state,
				D3D12_RESOURCE_STATE_STREAM_OUT |
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS |
				D3D12_RESOURCE_STATE_RENDER_TARGET |
				D3D12_RESOURCE_STATE_DEPTH_WRITE |
				D3D12_RESOURCE_STATE_COPY_DEST |
				D3D12_RESOURCE_STATE_RESOLVE_DEST |
				D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE |
				D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE |
				D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE
			);
		};

		inline static bool CanCombineResourceState(D3D12_RESOURCE_STATES stateA, D3D12_RESOURCE_STATES stateB)
		{
			return !HasWriteResourceState(stateA) && !HasWriteResourceState(stateB);
		}

		inline static bool IsTransitionAllowed(D3D12_COMMAND_LIST_TYPE commandlistType, D3D12_RESOURCE_STATES state)
		{
			constexpr int VALID_COMPUTE_QUEUE_RESOURCE_STATES =
				D3D12_RESOURCE_STATE_COMMON
				| D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				| D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				| D3D12_RESOURCE_STATE_COPY_DEST
				| D3D12_RESOURCE_STATE_COPY_SOURCE
				| D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

			constexpr int VALID_COPY_QUEUE_RESOURCE_STATES =
				D3D12_RESOURCE_STATE_COMMON
				| D3D12_RESOURCE_STATE_COPY_DEST
				| D3D12_RESOURCE_STATE_COPY_SOURCE;

			if (commandlistType == D3D12_COMMAND_LIST_TYPE_COMPUTE)
			{
				return (state & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == state;
			}
			else if (commandlistType == D3D12_COMMAND_LIST_TYPE_COPY)
			{
				return (state & VALID_COPY_QUEUE_RESOURCE_STATES) == state;
			}
			return true;
		}

		inline static bool NeedsTransition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES& after, bool allowCombine)
		{
			if (before == after)
				return false;

			// When resolving pending resource barriers, combining resource states is not working
			// This is because the last known resource state of the resource is used to update the resource
			// And so combining after_state during the result will result in the last known resource state not matching up.
			if (!allowCombine)
				return true;

			//Can read from 'write' DSV
			if (before == D3D12_RESOURCE_STATE_DEPTH_WRITE && after == D3D12_RESOURCE_STATE_DEPTH_READ)
				return false;

			if (after == D3D12_RESOURCE_STATE_COMMON)
				return before != D3D12_RESOURCE_STATE_COMMON;

			//Combine already transitioned bits
			if (D3D::CanCombineResourceState(before, after) && !EnumHasAllFlags(before, after))
				after |= before;

			return true;
		}

		inline void SetObjectName(ID3D12Object* pObject, const char* pName) noexcept
		{
			if (pObject)
				VERIFY_HR_EX(pObject->SetPrivateData(WKPDID_D3DDebugObjectName, (uint32_t)strlen(pName) + 1, pName), nullptr);
		}

		constexpr static const DXGI_FORMAT gDXGIFormatMap[] =
		{
			DXGI_FORMAT_UNKNOWN,

			DXGI_FORMAT_R8_UINT,
			DXGI_FORMAT_R8_SINT,
			DXGI_FORMAT_R8_UNORM,
			DXGI_FORMAT_R8_SNORM,
			DXGI_FORMAT_R8G8_UINT,
			DXGI_FORMAT_R8G8_SINT,
			DXGI_FORMAT_R8G8_UNORM,
			DXGI_FORMAT_R8G8_SNORM,
			DXGI_FORMAT_R16_UINT,
			DXGI_FORMAT_R16_SINT,
			DXGI_FORMAT_R16_UNORM,
			DXGI_FORMAT_R16_SNORM,
			DXGI_FORMAT_R16_FLOAT,
			DXGI_FORMAT_B4G4R4A4_UNORM,
			DXGI_FORMAT_B5G6R5_UNORM,
			DXGI_FORMAT_B5G5R5A1_UNORM,
			DXGI_FORMAT_R8G8B8A8_UINT,
			DXGI_FORMAT_R8G8B8A8_SINT,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R8G8B8A8_SNORM,
			DXGI_FORMAT_B8G8R8A8_UNORM,
			DXGI_FORMAT_R10G10B10A2_UNORM,
			DXGI_FORMAT_R11G11B10_FLOAT,
			DXGI_FORMAT_R16G16_UINT,
			DXGI_FORMAT_R16G16_SINT,
			DXGI_FORMAT_R16G16_UNORM,
			DXGI_FORMAT_R16G16_SNORM,
			DXGI_FORMAT_R16G16_FLOAT,
			DXGI_FORMAT_R32_UINT,
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R16G16B16A16_UINT,
			DXGI_FORMAT_R16G16B16A16_SINT,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_UNORM,
			DXGI_FORMAT_R16G16B16A16_SNORM,
			DXGI_FORMAT_R32G32_UINT,
			DXGI_FORMAT_R32G32_SINT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_UINT,
			DXGI_FORMAT_R32G32B32_SINT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_UINT,
			DXGI_FORMAT_R32G32B32A32_SINT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,

			DXGI_FORMAT_BC1_UNORM,
			DXGI_FORMAT_BC2_UNORM,
			DXGI_FORMAT_BC3_UNORM,
			DXGI_FORMAT_BC4_UNORM,
			DXGI_FORMAT_BC4_SNORM,
			DXGI_FORMAT_BC5_UNORM,
			DXGI_FORMAT_BC5_SNORM,
			DXGI_FORMAT_BC6H_UF16,
			DXGI_FORMAT_BC6H_SF16,
			DXGI_FORMAT_BC7_UNORM,

			DXGI_FORMAT_D16_UNORM,
			DXGI_FORMAT_D32_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
		};

		constexpr [[nodiscard]] DXGI_FORMAT ConvertFormat(ResourceFormat format) noexcept
		{
			return gDXGIFormatMap[(uint32_t)format];
		}
	}
}