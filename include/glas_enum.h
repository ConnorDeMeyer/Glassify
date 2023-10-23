#pragma once
#include <type_traits>

/**
 * This is a utility header file for enum classes that will enable bit operations on enums.
 * To enable this feature on any enum use the GLAS_ENUM_BIT(ENUM) macro
 */

template <typename Enum>
inline constexpr bool EnumEnableBitOperations = false;

template <typename Enum>
concept BitFieldOperators = EnumEnableBitOperations<Enum> && std::is_enum_v<Enum>;

template <typename Enum>
constexpr Enum operator|(Enum lhs, Enum rhs) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template <typename Enum>
constexpr Enum operator&(Enum lhs, Enum rhs) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template <typename Enum>
constexpr Enum operator^(Enum lhs, Enum rhs) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

template <typename Enum>
constexpr Enum operator~(Enum val) requires BitFieldOperators<Enum>
{
	using T = std::underlying_type_t<Enum>;
	return static_cast<Enum>(~static_cast<T>(val));
}

template <typename Enum>
constexpr bool operator!(Enum val) requires BitFieldOperators<Enum>
{
	return !static_cast<bool>(val);
}

#define GLAS_ENUM_BIT(ENUM) template <> inline constexpr bool EnumEnableBitOperations<ENUM> = true;

