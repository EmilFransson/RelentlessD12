#pragma once

#if defined RLS_BUILD_DLL
	#define RLS_API __declspec(dllexport)
#else
	#define RLS_API __declspec(dllimport)
#endif
