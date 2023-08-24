#pragma once
#include <cstdint>
#include "glas_enum.h"

namespace glas
{
	/**
	* PROPERTIES
	*/

	using EnumBase = uint32_t;

	// Member Properties

	using MemberEnumBase = EnumBase;

	enum class MemberProperties : EnumBase
	{
		None = 0,
		Serializable = 1 << 0,
	};

	inline constexpr MemberProperties DefaultMemberProperties{
		MemberProperties::Serializable
	};

	// Function Properties

	using FunctionEnumBase = EnumBase;

	enum class FunctionProperties : EnumBase
	{
		None = 0,
		Method = 1 << 0,
		ConstMethod = 1 << 1,
		ServerCallable = 1 << 16,
		ClientCallable = 1 << 17,
		ScriptCallable = 1 << 18,
	};

	inline constexpr FunctionProperties DefaultFunctionProperties{
		FunctionProperties::None
	};
}

GLAS_ENUM_BIT(glas::MemberProperties);
GLAS_ENUM_BIT(glas::FunctionProperties);