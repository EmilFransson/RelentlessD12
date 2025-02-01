#pragma once
namespace Relentless
{
#define RLS Relentless

#ifndef DELETE_COPY_CONSTRUCTOR
#define DELETE_COPY_CONSTRUCTOR(T)	\
			T(const T&) = delete;	\
			T& operator=(const T&) = delete;
#endif
#ifndef DELETE_MOVE_CONSTRUCTOR
#define DELETE_MOVE_CONSTRUCTOR(T)	\
			T(T&&) = delete;	\
			T& operator=(const T&&) = delete;
#endif
#ifndef DELETE_COPY_MOVE_CONSTRUCTOR
#define DELETE_COPY_MOVE_CONSTRUCTOR(T)	\
			DELETE_COPY_CONSTRUCTOR(T);	\
			DELETE_MOVE_CONSTRUCTOR(T);
#endif
#ifndef DELETE_DEFAULT_CONSTRUCTOR
#define DELETE_DEFAULT_CONSTRUCTOR(T)	\
			T() = delete;	\
			~T() = delete;
#endif
#ifndef STATIC_CLASS
#define STATIC_CLASS(T)	\
			DELETE_DEFAULT_CONSTRUCTOR(T)	\
			DELETE_COPY_CONSTRUCTOR(T)	\
			DELETE_MOVE_CONSTRUCTOR(T)
#endif

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

	#ifndef DEBUG_OPERATION
		#define DEBUG_OPERATION(statement) statement
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

#ifndef DEBUG_OPERATION
	#define DEBUG_OPERATION(statement)
#endif
#endif
}