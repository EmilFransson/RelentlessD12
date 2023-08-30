#include "Helper.h"

namespace Relentless
{
	UUID CreateUUID() noexcept
	{
		UUID uuID;
		#if defined RLS_DEBUG
		RLS_ASSERT(::UuidCreate(&uuID) == RPC_S_OK, "Failed to generate UUID.");
		#else
		::UuidCreate(&uuID);
		#endif

		return uuID;
	}
}