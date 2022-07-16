#pragma once
namespace Relentless
{
#define RLS Relentless

#if defined(RLS_DEBUG)
	#ifndef APPLICATION_SUFFIX
		#define APPLICATION_SUFFIX "--Debug"
	#endif

	#ifndef INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION
		#define INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	#ifndef RLS_NEW
		#define RLS_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	#endif
#else
	#ifndef APPLICATION_SUFFIX
		#define APPLICATION_SUFFIX ""
	#endif

	#ifndef INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION
		#define INITIALIZE_DEBUG_MEMORY_LEAK_DETECTION 
	#endif

	#ifndef RLS_NEW
		#define RLS_NEW new
	#endif
#endif
}