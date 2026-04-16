#pragma once

#define NO_DISCARD [[nodiscard]]
#define MAYBE_UNUSED [[maybe_unused]]

typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

template<typename T>
using WeakPtr = std::weak_ptr<T>;

template<typename T, typename... Args>
UniquePtr<T> MakeUnique(Args&&... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

using Path			= std::filesystem::path;
using String		= std::string;
using StringView	= std::string_view;
using WideString	= std::wstring;

#define DECLARE_BITMASK_TYPE(Enum) \
inline constexpr Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
inline constexpr Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
inline constexpr Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
inline constexpr bool  operator! (Enum  E) { return !(__underlying_type(Enum))E; } \
inline constexpr Enum  operator~ (Enum  E) { return (Enum)~(__underlying_type(Enum))E; }

template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr inline bool EnumHasAllFlags(Enum Flags, Enum Contains)
{
	return (((__underlying_type(Enum))Flags) & (__underlying_type(Enum))Contains) == ((__underlying_type(Enum))Contains);
}

template<typename Enum, typename = std::enable_if_t<std::is_enum_v<Enum>>>
constexpr inline bool EnumHasAnyFlags(Enum Flags, Enum Contains)
{
	return (((__underlying_type(Enum))Flags) & (__underlying_type(Enum))Contains) != 0;
}

namespace Relentless
{
	template<typename T>
	struct Is_Pointer_Like : std::is_pointer<T> {};
}

