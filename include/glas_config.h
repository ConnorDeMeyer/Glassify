#pragma once

#include <functional>
#include <bitset>
#include <set>

#include "glas_decl.h"

/**
 * Comment to disable Addon and unconmment to enable
 */

#define GLAS_STORAGE
#define GLAS_SERIALIZATION

/**
 * struct holds type information about each type
 */

namespace glas
{
	struct TypeInfo final
	{
		std::string_view								Name				{ };
		uint32_t										Size				{ };
		uint32_t										Align				{ };
		void*											VTable				{ };

		std::vector<MemberInfo>							Members				{ };
		std::unordered_map<FunctionId, FunctionInfo>	MemberFunctions		{ };
		std::unordered_map<std::string_view, FunctionId>MemberFunctionNames	{ };

		std::vector<BaseClassInfo>						BaseClasses			{ };
		std::vector<TypeId>								ChildClasses		{ };

#ifdef GLAS_STORAGE
		void (*Constructor)			(void*)									{ };
		void (*MoveConstructor)		(void*, void*)							{ };
		void (*CopyConstructor)		(void*, const void*)					{ };
		void (*Destructor)			(void*)									{ };
		void (*Swap)				(void*, void*)							{ };
#endif // GLASS_STORAGE
#ifdef GLAS_SERIALIZATION
		void (*Serializer)			(std::ostream&, const void*)			{ };
		void (*BinarySerializer)	(std::ostream&, const void*)			{ };
		void (*Deserializer)		(std::istream&, void*)					{ };
		void (*BinaryDeserializer)	(std::istream&, void*)					{ };
#endif // GLAS_SERIALIZATION

		/**
		 * Add custom Type Information variables here
		 */



		/**
		 * End of Custom Type Information Variables
		 */

		template <typename T>
		static TypeInfo Create();
	};
}

#ifdef GLAS_STORAGE
#include "storage/glas_storage_config.h"
#endif

#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization_config.h"
#endif

/**
 * Type Info Variables Initialization
 */

#include "glas_dependencies.h"

namespace glas
{
	template <>
	TypeInfo TypeInfo::Create<void>()
	{
		TypeInfo info{};
		info.Name = TypeName<void>();
		return info;
	}

	template<typename T>
	TypeInfo TypeInfo::Create()
	{
		AddDependency<T> Dependency{};

		TypeInfo info{};

		info.Name = TypeName<T>();
		if constexpr (!std::is_same_v<void, T>)
		{
			info.Size = sizeof(T);
			info.Align = alignof(T);
		}

		if constexpr (std::is_polymorphic_v<T> && std::is_default_constructible_v<T>)
		{
			T instance{};
			info.VTable = *reinterpret_cast<void**>(&instance);
		}

#ifdef GLAS_STORAGE
		Storage::FillInfo<T>(info);
#endif 

#ifdef GLAS_SERIALIZATION
		Serialization::FillInfo<T>(info);
#endif 

		/**
		 * Begin initialization of Custom Information variables here
		 */



		/**
		 * End  initialization of Custom Type Information Variables
		 */

		return info;
	}
}



#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization.h"
#endif // GLAS_SERIALIZATION
#ifdef GLAS_STORAGE
#include "storage/glas_storage.h"
#endif // GLAS_SERIALIZATION