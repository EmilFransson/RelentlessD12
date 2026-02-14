#pragma once
#include <Relentless.h>
#include "ImGui/ImGuiIncludes.h"

namespace Relentless
{
	namespace ImGuiHelpers
	{
		template<typename DataType>
		NO_DISCARD constexpr ImGuiDataType ToImGuiDataType() noexcept
		{
			using T = std::remove_cv_t<std::remove_reference_t<DataType>>;

			if constexpr (std::is_same_v<T, char>)
				return ImGuiDataType_::ImGuiDataType_S8;
			else if constexpr (std::is_same_v<T, short>)
				return ImGuiDataType_::ImGuiDataType_S16;
			else if constexpr (std::is_same_v<T, int32>)
				return ImGuiDataType_::ImGuiDataType_S32;
			else if constexpr (std::is_same_v<T, int64>)
				return ImGuiDataType_::ImGuiDataType_S64;
			else if constexpr (std::is_same_v<T, unsigned char>)
				return ImGuiDataType_::ImGuiDataType_U8;
			else if constexpr (std::is_same_v<T, unsigned short>)
				return ImGuiDataType_::ImGuiDataType_U16;
			else if constexpr (std::is_same_v<T, uint32>)
				return ImGuiDataType_::ImGuiDataType_U32;
			else if constexpr (std::is_same_v<T, uint64>)
				return ImGuiDataType_::ImGuiDataType_U64;
			else if constexpr (std::is_same_v<T, float>)
				return ImGuiDataType_::ImGuiDataType_Float;
			else if constexpr (std::is_same_v<T, double>)
				return ImGuiDataType_::ImGuiDataType_Double;
			else if constexpr (std::is_same_v<T, String>)
				return ImGuiDataType_::ImGuiDataType_String;
			else
			{
				RLS_ASSERT(false, "[ImGuiHelpers::ToImGuiDataType]: Unsupported data type encountered.");
				return ImGuiDataType_::ImGuiDataType_Float;
			}
		}

		template<typename DataType>
		NO_DISCARD constexpr const char* GetTypeFormat() noexcept
		{
			using T = std::remove_cv_t<std::remove_reference_t<DataType>>;

			if constexpr (std::is_same_v<T, char>)
				return "%c";
			else if constexpr (std::is_same_v<T, short>)
				return "%hd";
			else if constexpr (std::is_same_v<T, int32>)
				return "%d";
			else if constexpr (std::is_same_v<T, int64>)
				return "%lld";
			else if constexpr (std::is_same_v<T, unsigned char>)
				return "%c";
			else if constexpr (std::is_same_v<T, unsigned short>)
				return "%hu";
			else if constexpr (std::is_same_v<T, uint32>)
				return "%u";
			else if constexpr (std::is_same_v<T, uint64>)
				return "%llu";
			else if constexpr (std::is_same_v<T, float>)
				return "%.2f";
			else if constexpr (std::is_same_v<T, double>)
				return "%.2f";
			else if constexpr (std::is_same_v<T, String>)
				return "%s";
			else
			{
				RLS_ASSERT(false, "[ImGuiHelpers::ToImGuiDataType]: Unsupported data type encountered.");
				return "%f";
			}
		}
	}
}