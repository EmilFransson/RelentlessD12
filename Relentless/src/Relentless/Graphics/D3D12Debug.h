#pragma once
#include "../Core.h"
namespace Relentless
{
	class D3D12Debug
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12InfoQueue>& GetInfoQueue() noexcept { return m_pInfoQueue; }
	private:
		D3D12Debug() noexcept = default;
		~D3D12Debug() noexcept = default;
	private:
		static Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_pInfoQueue;
	};
}

#if defined(RLS_DEBUG)
	#ifndef DXCall
		#define DXCall(function)	\
		{	\
			if (D3D12Debug::GetInfoQueue() != nullptr)	\
				{	\
					D3D12Debug::GetInfoQueue()->ClearStoredMessages();	\
				}	\
			HRESULT hr = (function);	\
			if (FAILED(hr))	\
			{	\
				std::stringstream ss;	\
				ss << "Direct3D12 has encountered a critical error.\n";	\
				ss << "File: " << __FILE__ << "\n";	\
				ss << "Function: " << __FUNCTION__ << "\n";	\
				ss << "Line: " << __LINE__ << "\n";	\
				if (D3D12Debug::GetInfoQueue() != nullptr)	\
				{	\
					for (uint32_t messageIndex{0u}; messageIndex < D3D12Debug::GetInfoQueue()->GetNumStoredMessages(); messageIndex++)	\
						{	\
							SIZE_T messageLength{0u};	\
							D3D12Debug::GetInfoQueue()->GetMessage(messageIndex, NULL, &messageLength);	\
							std::unique_ptr<D3D12_MESSAGE> pMessage = std::unique_ptr<D3D12_MESSAGE>(RLS_NEW D3D12_MESSAGE[messageLength]);	\
							D3D12Debug::GetInfoQueue()->GetMessage(messageIndex, pMessage.get(), &messageLength);	\
							ss << "D3D12 Message [" << std::to_string(messageIndex + 1) << "]: ";	\
							ss << pMessage->pDescription;	\
							ss << "\n\n";	\
						}	\
				}	\
				RLS_ASSERT(false, ss.str());	\
			}	\
		}
	#endif //DXCall

	#ifndef DXCall_STD
		#define DXCall_STD(function2)	\
		{	\
			D3D12Debug::GetInfoQueue()->ClearStoredMessages();	\
			(function2);	\
			if (D3D12Debug::GetInfoQueue()->GetNumStoredMessages() > 0)	\
			{	\
				std::stringstream ss2;	\
				ss2 << "Direct3D12 has encountered a critical error.\n";	\
				ss2 << "File: " << __FILE__ << "\n";	\
				ss2 << "Function: " << __FUNCTION__ << "\n";	\
				ss2 << "Line: " << __LINE__ << "\n";	\
				for (uint32_t messageIndex2{0u}; messageIndex2 < D3D12Debug::GetInfoQueue()->GetNumStoredMessages(); messageIndex2++)	\
				{	\
					SIZE_T messageLength2{0u};	\
					D3D12Debug::GetInfoQueue()->GetMessage(messageIndex2, NULL, &messageLength2);	\
					std::unique_ptr<D3D12_MESSAGE> pMessage2 = std::unique_ptr<D3D12_MESSAGE>(RLS_NEW D3D12_MESSAGE[messageLength2]);	\
					D3D12Debug::GetInfoQueue()->GetMessage(messageIndex2, pMessage2.get(), &messageLength2);	\
					ss2 << "D3D12 Message [" << std::to_string(messageIndex2 + 1) << "]: ";	\
					ss2 << pMessage2->pDescription;	\
				}	\
				RLS_ASSERT(false, ss2.str());	\
			}	\
		}
	#endif //DXCall_STD

	#ifndef NAME_D12_OBJECT
		#define NAME_D12_OBJECT(obj, name) DXCall(obj->SetName(name)); 
	#endif //NAME_D12_OBJECT
	#ifndef NAME_D12_OBJECT_INDEXED
		#define NAME_D12_OBJECT_INDEXED(obj, name, index)	\
			{	\
				std::wstring finalName = name + L'[' + std::to_wstring(index) + L']';	\
				DXCall(obj->SetName(finalName.c_str()));	\
			}
	#endif //NAME_D12_OBJECT_INDEXED
#else
	#ifndef DXCall
		#define DXCall(function) function
	#endif //DXCall

	#ifndef DXCall_STD
		#define DXCall_STD(function) function
	#endif //DXCall_STD

	#ifndef NAME_D12_OBJECT
		#define NAME_D12_OBJECT(obj, name) 
	#endif //NAME_D12_OBJECT

	#ifndef NAME_D12_OBJECT_INDEXED
		#define NAME_D12_OBJECT_INDEXED(obj, name, index) 
	#endif //NAME_D12_OBJECT_INDEXED
#endif