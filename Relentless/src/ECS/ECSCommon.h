#pragma once
namespace Relentless
{
	class EntityManager;

	/*! @brief Invalid entity alias. */
	#define NULL_ENTITY (std::numeric_limits<uint32_t>::max() << 12)

	/*! @brief Opaque entity type. */
	using entity = uint32_t;
}