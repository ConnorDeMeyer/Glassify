#pragma once
#include <cstdint>
#include "glas_enum.h"

namespace glas
{
	/**
	* PROPERTIES
	*/

	using EnumBase = uint32_t;
	using MemberEnumBase = EnumBase;
	using FunctionEnumBase = EnumBase;

	/**
	 * Properties for member variables that can be freely customized by the developer.
	 * These properties are set when a Member Variable is registered to the reflection system.
	 * @see GlasAutoRegisterMember
	 * @see GLAS_MEMBER
	 */
	enum class MemberProperties : EnumBase
	{
		/** Required properties */
		None = 0,
		Serializable = 1 << 0,
	};

	/**
	 * Default properties for member variables.
	 * @see GLAS_MEMBER_DEF
	 */
	inline constexpr MemberProperties DefaultMemberProperties{
		MemberProperties::Serializable
	};


	/**
	 * Properties for functions and methods that can be freely customized by the developer.
	 * These properties are set when a Function or Method is registered to the reflection system.
	 * @see GlasAutoRegisterFunction
	 * @see GlasAutoRegisterMemberFunction
	 * @see GLAS_FUNCTION
	 * @see GLAS_MEMBER_FUNCTION
	 */
	enum class FunctionProperties : EnumBase
	{
		/** Required properties */
		None = 0,
		Method = 1 << 0,
		ConstMethod = 1 << 1,

		/** Custom properties example*/
		ServerCallable = 1 << 16,
		ClientCallable = 1 << 17,
		ScriptCallable = 1 << 18,
	};

	/**
	 * Default properties for functions and methods.
	 * @see GLAS_FUNCTION_DEF
	 * @see GLAS_MEMBER_FUNCTION_DEF
	 */
	inline constexpr FunctionProperties DefaultFunctionProperties{
		FunctionProperties::None
	};
}

GLAS_ENUM_BIT(glas::MemberProperties);
GLAS_ENUM_BIT(glas::FunctionProperties);