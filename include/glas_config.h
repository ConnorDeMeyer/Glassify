#pragma once

#include <functional>
#include <bitset>
#include <set>

#include "glas_decl.h"

namespace glas
{

/**
 * Comment to disable Addons and uncomment to enable
 */

#define GLAS_STORAGE
#define GLAS_SERIALIZATION


/**
 * TypeInfo holds information about each type
 */

	struct TypeInfo final
	{
		/** Name of the type. */
		std::string					Name				{ };

		/**
		 * Size of the type gathered using sizeof().
		 * @see sizeof
		 */
		uint32_t					Size				{ };

		/**
		 * Alignment of the type gathered using alignof()
		 * @see alignof
		 */
		uint32_t					Align				{ };

		/**
		 * v-table pointer of the type in case the type is polymorphic
		 * @warning experimental
		 */
		void*						VTable				{ };

		/**
		 * Member variables that have been registered to this type.
		 * @see MemberInfo
		 * @see AutoRegisterMember
		 * @see GLAS_MEMBER
		 */
		std::vector<MemberInfo>		Members				{ };

		/**
		 * Member Functions that have been registered to this type.
		 * @see FunctionId
		 * @see AutoRegisterFunction
		 * @see GLAS_FUNCTION
		 */
		std::vector<FunctionId>		MemberFunctions		{ };

		/**
		 * Base classes of this type that have been registered.
		 * @see BaseClassInfo
		 * @see AutoRegisterChildOnce
		 * @see GLAS_CHILD
		 */
		std::vector<BaseClassInfo>	BaseClasses			{ };

		/**
		 * Child classes of this type that have been registered.
		 * @see TypeId
		 * @see AutoRegisterChildOnce
		 * @see GLAS_CHILD
		 */
		std::vector<TypeId>			ChildClasses		{ };

#ifdef GLAS_STORAGE
		/**
		 * Function pointer that constructs the type in place at the given address.
		 * @param 0 address for construction
		 */
		void (*Constructor)			(void*)									{ };

		/**
		 * Function pointer that constructs the type in place at the given address using the copy constructor.
		 * @param 0 address for construction
		 * @param 1 address of copyable type instance
		 * @see EnableCopyConstructor
		 */
		void (*CopyConstructor)		(void*, const void*)					{ };

		/**
		 * Function pointer that constructs the type in place at the given address using the move constructor.
		 * @param 0 address for construction
		 * @param 1 address of moveable type instance
		 * @see EnableMoveConstructor
		 */
		void (*MoveConstructor)		(void*, void*)							{ };

		/**
		 * Function pointer that destructs the type at the given address
		 * @param 0 address for destruction
		 */
		void (*Destructor)			(void*)									{ };

		/**
		 * Function pointer that swaps two instances of a type
		 * @param 0 address of type instance 1
		 * @param 1 address of type instance 2
		 */
		void (*Swap)				(void*, void*)							{ };
#endif // GLASS_STORAGE

#ifdef GLAS_SERIALIZATION
		/**
		 * Function pointer that serializes the given type instance to a stream in json format
		 * @param 0 out stream for serialization
		 * @param 1 address of type instance that will be serialized
		 * @see Serialize
		 * @see GlasSerialize
		 */
		void (*Serializer)			(std::ostream&, const void*)			{ };

		/**
		 * Function pointer that serialized the given type instance to a stream in binary format
		 * @param 0 out stream for serialization
		 * @param 1 address of type instance that will be serialized
		 * @see SerializeBinary
		 * @see GlasSerializeBinary
		 */
		void (*BinarySerializer)	(std::ostream&, const void*)			{ };

		/**
		 * Function pointer that initializes a type instance using a stream in json format
		 * @param 0 in stream containing data of type
		 * @param 1 address of type instance that will be initialized
		 * @see Deserialize
		 * @see GlasDeserialize
		 */
		void (*Deserializer)		(std::istream&, void*)					{ };

		/**
		 * Function pointer that initialized a type instance using a stream in binary format
		 * @param 0 in stream containing data of type
		 * @param 1 address of type instance that will be initialized
		 * @see DeserializeBinary
		 * @see GlasDeserializeBinary
		 */
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
	/** specialized TypeInfo creator for type void*/
	template <>
	inline TypeInfo TypeInfo::Create<void>()
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
		info.Size = sizeof(T);
		info.Align = alignof(T);

		if constexpr (std::is_polymorphic_v<T> && std::is_default_constructible_v<T>)
		{
			T instance{};
			info.VTable = *reinterpret_cast<void**>(&instance);
		}

#ifdef GLAS_STORAGE
		Storage::FillTypeInfo<T>(info);
#endif 

#ifdef GLAS_SERIALIZATION
		Serialization::FillTypeInfo<T>(info);
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