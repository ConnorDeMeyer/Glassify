#pragma once

/**
 * GLASSIFY ADDONS
 */

//#define GLAS_STORAGE
//#define GLAS_SERIALIZATION_BINARY
//#define GLAS_SERIALIZATION_JSON
//#define GLAS_SERIALIZATION_YAML

#include <span>
#include <array>
#include <cassert>
#include <tuple>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <functional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

/** External dependencies*/
#ifdef GLAS_SERIALIZATION_JSON
#include "serialization/Json3rdParty/rapidjson/rapidjson.h"
#include "serialization/Json3rdParty/rapidjson/document.h"
#endif

#ifdef GLAS_SERIALIZATION_YAML
#include "serialization/YAML3rdParty/yaml.h"
#endif


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

namespace glas
{
	//https://stackoverflow.com/questions/35941045/can-i-obtain-c-type-names-in-a-constexpr-way
	//asnwer by einpoklum

	/**
	 * Returns a string_view containing the name of the type at compile time
	 */
	template <typename T> constexpr std::string_view TypeName();

	template <>
	constexpr std::string_view TypeName<void>()
	{
		return "void";
	}

	namespace detail
	{
		using type_name_prober = void;

		template <typename T>
		constexpr std::string_view wrapped_type_name()
		{
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
		}

		constexpr std::size_t wrapped_type_name_prefix_length()
		{
			return wrapped_type_name<type_name_prober>().find(TypeName<type_name_prober>());
		}

		constexpr std::size_t wrapped_type_name_suffix_length()
		{
			return wrapped_type_name<type_name_prober>().length()
				- wrapped_type_name_prefix_length()
				- TypeName<type_name_prober>().length();
		}
	}

	template <typename T>
	constexpr std::string_view TypeName()
	{
		constexpr auto wrapped_name = detail::wrapped_type_name<T>();
		constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
		constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
		constexpr auto type_name_length = wrapped_name.length() - prefix_length - suffix_length;
		return wrapped_name.substr(prefix_length, type_name_length);
	}

	/**
	 * Hashes a string view into a uint64_t at compile time
	 */
	constexpr uint64_t hash(std::string_view str);

	/**
	 * Hashes a span of uint64_t into a single uint64_t at compile time
	 */
	constexpr uint64_t hash(std::span<const uint64_t> span);

	/**
	 * Creates a hash for any given type
	 */
	template <typename Type>
	constexpr uint64_t TypeHash()
	{
		return hash(TypeName<Type>());
	}

	/**
	 * Counts the amount of pointers the given type has
	 */
	template <typename T>
	constexpr uint32_t CountPointers(uint32_t counter = 0)
	{
		if constexpr (std::is_pointer_v<T>)
			return CountPointers<std::remove_pointer_t<T>>(++counter);
		else
			return counter;
	}

	/**
	 * Removes the pointers from a type.
	 * @source https://stackoverflow.com/questions/9851594/standard-c11-way-to-remove-all-pointers-of-a-type
	 */
	template <typename T> struct remove_all_pointers
	{
		using Type = T;
	};

	template <typename T> struct remove_all_pointers<T*>
	{
		using Type = typename remove_all_pointers<T>::Type;
	};

	template <typename T>
	using remove_all_pointers_t = typename remove_all_pointers<T>::Type;

	/**
	 * Strips the given type of all const, pointers, references, extents.
	 * const int**& -> int
	 */
	template <typename T> struct strip_type
	{
		using Type = std::remove_cvref_t<remove_all_pointers_t<std::remove_reference_t<std::remove_all_extents_t<T>>>>;
	};

	template <typename T>
	using strip_type_t = typename strip_type<T>::Type;

	/**
	 * FORWARD DECLARATIONS
	 */

	struct	TypeInfo;
	class	TypeId;
	class	VariableId;
	struct	MemberInfo;

	namespace Storage
	{
		class TypeTuple;
	}

	/**
	 * Class that contains an 64bit integer that is used as an identifier for a type.
	 * @see Create
	 */
	class TypeId final
	{
	public:
		constexpr TypeId	()	= default;
		constexpr ~TypeId	()	= default;

		constexpr TypeId(uint64_t id) : m_ID{ id } {}

		constexpr TypeId(const TypeId&) = default;
		constexpr TypeId(TypeId&&) noexcept = default;
		constexpr TypeId& operator=(const TypeId&) = default;
		constexpr TypeId& operator=(TypeId&&) noexcept = default;

	public:
		/**
		 * Static templated constexpr function that creates a TypeId from the given Type T.
		 * @return TypeId of the type at compile time.
		 */
		template <typename T>
		static constexpr TypeId Create();

	public:
		/**
		 * Returns the type information associated with this type.
		 * Will assert if the type is not registered.
		 * @see RegisterType()
		 * @see TypeInfo
		 * @return Information about the type.
		 */
		const TypeInfo&			GetInfo		()					const;
		constexpr void			SetTypeId	(uint64_t typeId)			{ m_ID = typeId; }
		constexpr uint64_t		GetId		()					const	{ return m_ID; }
		constexpr bool			IsValid		()					const	{ return m_ID; }

		/**
		 * Returns the info of the member variable that is found at the given offset (offsetof()).
		 * @param offset the offset of the member variable
		 * @see offsetof
		 */
		const MemberInfo*		GetMemberInfo(size_t offset)		const;

	private:
		uint64_t m_ID{};
	};

	/**
	 * VARIABLE REFLECTION
	 */

	/**
	 * Class that contains a TypeId and flags to describe what kind of variable it is.
	 * Variables can contain modifiers like: const, volatile, &, &&, *, [].
	 * VariableId has flags that describe these modifiers.
	 * @see Create
	 */
	class VariableId final
	{
	private:

		static constexpr uint32_t ConstFlag			= 1 << 0; /**< Flag that marks if the variable is const*/
		static constexpr uint32_t ReferenceFlag		= 1 << 1; /**< Flag that marks if the variable is a reference (&)*/
		static constexpr uint32_t VolatileFlag		= 1 << 2; /**< Flag that marks if the variable is volatile*/
		static constexpr uint32_t RValReferenceFlag = 1 << 3; /**< Flag that marks if the variable is a right value reference (&&)*/

	public:

		constexpr explicit VariableId(TypeId id) : m_Type{ id } {}
		constexpr VariableId() = default;

		/**
		 * Creates a VariableId with the given type at compile time.
		 */
		template <typename T>
		static constexpr VariableId Create();

	public:

		constexpr TypeId	GetTypeId				()				const	{ return m_Type; }
		constexpr void		SetTypeId				(TypeId id)				{ m_Type = id; }

		constexpr void		SetConstFlag			()						{ m_TraitFlags |= ConstFlag; }
		constexpr void		SetReferenceFlag		()						{ m_TraitFlags |= ReferenceFlag; }
		constexpr void		SetVolatileFlag			()						{ m_TraitFlags |= VolatileFlag; }
		constexpr void		SetRValReferenceFlag	()						{ m_TraitFlags |= RValReferenceFlag; }

		constexpr void		RemoveConstFlag			()						{ m_TraitFlags &= ~ConstFlag; }
		constexpr void		RemoveReferenceFlag		()						{ m_TraitFlags &= ~ReferenceFlag; }
		constexpr void		RemoveVolatileFlag		()						{ m_TraitFlags &= ~VolatileFlag; }
		constexpr void		RemoveRValReferenceFlag	()						{ m_TraitFlags &= ~RValReferenceFlag; }

		constexpr void		SetPointerAmount		(uint16_t amount)		{ m_PointerAmount = amount; }
		constexpr uint32_t	GetPointerAmount		()				const	{ return m_PointerAmount; }

		constexpr void		SetArraySize			(uint32_t Size)			{ m_ArraySize = Size; }
		constexpr uint32_t	GetArraySize			()				const	{ return m_ArraySize; }

		constexpr bool		IsConst					()				const	{ return m_TraitFlags & ConstFlag; }
		constexpr bool		IsReference				()				const	{ return m_TraitFlags & ReferenceFlag; }
		constexpr bool		IsVolatile				()				const	{ return m_TraitFlags & VolatileFlag; }
		constexpr bool		IsRValReference			()				const	{ return m_TraitFlags & RValReferenceFlag; }
		constexpr bool		IsPointer				()				const	{ return m_PointerAmount; }
		constexpr bool		IsArray					()				const	{ return m_ArraySize == 1; }
		constexpr bool		IsRefOrPointer			()				const	{ return IsPointer() || IsReference() || IsRValReference(); }

		/**
		 * Hashes the VariableId into a hash that fits into a uint64_t
		 */
		constexpr uint64_t	GetHash					()				const	{ return m_Type.GetId() ^ m_ArraySize ^ (static_cast<uint64_t>(m_PointerAmount) << 32) ^ (static_cast<uint64_t>(m_TraitFlags) << 40); }

		constexpr uint32_t	GetSize					()				const;
		constexpr uint32_t	GetAlign				()				const;

		friend constexpr bool operator==(const VariableId& lhs, const VariableId& rhs);
		friend std::ostream& operator<<(std::ostream& lhs, const VariableId& rhs);
		friend std::istream& operator>>(std::istream& lhs, const VariableId& rhs);

		/**
		 * Returns the literal representation of the variable
		 */
		std::string			ToString				()				const;

	private:

		TypeId		m_Type			{ };	/**< The underlying type id */
		uint32_t	m_ArraySize		{ };	/**< if the variable is a fixed sized array, the size will be contained in this. else it will be 1 */
		uint16_t	m_PointerAmount	{ };	/**< The amount of pointers that are attached to the Type */
		uint8_t		m_TraitFlags	{ };	/**< Other flags (const, volatile, reference, RValReference) */
	};

	/**
	 * MEMBER VARIABLE REFLECTION
	 */

	/**
	 * Structure that describes information about member variables of a class or struct
	 */
	struct MemberInfo final
	{
		std::string			Name		{ }; /**< Name of the member variable*/
		VariableId			Variable	{ }; /**< Type Information about the member variable*/
		uint32_t			Offset		{ }; /**< Offset of the member variable inside of the owning class*/
		uint32_t			Size		{ }; /**< Size of the member variable inside of the owning class*/
		uint32_t			Align		{ }; /**< Alignment of the member variable inside of the owning class*/

		/**
		 * Custom properties that can be set when registering the member variables.
		 * @see MemberProperties
		 * @see glas_properties.h
		 */
		MemberProperties	Properties	{ }; 

		constexpr bool operator<(const MemberInfo& rhs) const { return Offset < rhs.Offset; }

		/**
		 * Check whether a property is set in this member variable
		 * @see MemberProperties
		 * @see Properties
		 */
		constexpr bool IsPropertySet(MemberProperties property) const;
	};

	/**
	 * FUNCTION REFLECTION
	 */

	/**
	 * Structure that described information about a function or methods (member function).
	 */
	struct FunctionInfo final
	{
		const void*						FunctionAddress	{ }; /**< Address of the function*/
		VariableId						ReturnType		{ }; /**< Information about the return type of the function*/
		std::string						Name			{ }; /**< Name of the function*/
		std::vector<VariableId>			ParameterTypes	{ }; /**< Vector containing information about each parameter*/

		/**
		 * Hash of the return type and parameter types combined.
		 * Is used to quickly cast from the function address into a proper function type.
		 * @see Cast
		 * @see GetTypeHash
		 * @see VariableId
		 */
		uint64_t						TypesHash		{ };

		/**
		 * The type that owns this function in case the function is a method.
		 * @see IsMethod
		 * @see TypeId
		 */
		TypeId							OwningType		{ };

		/**
		 * Custom Properties that can be set when registering the function or method.
		 * @see FunctionProperties
		 * @see glas_properties.h
		 */
		FunctionProperties				Properties		{ };

		/**
		 * Function pointer containing a call to a lambda that will execute the registered function with the parameters inside a TypeTuple.
		 * @param 0 address of the function
		 * @param 1 TypeTuple containing the data of the parameters to call the function
		 * @param 2 optional address of a uninitialized variable that will be set when the function returns a value.
		 * @see Call
		 * @see TypeTuple
		 */
		void(*FunctionCaller)(const void*, Storage::TypeTuple&, void*);

		/**
		 * Function pointer containing a call to a lambda that will execute the registered method with the parameters inside a TypeTuple.
		 * @param 0 address of the function
		 * @param 1 address instance of the owning type
		 * @param 2 TypeTuple containing the data of the parameters to call the function
		 * @param 3 optional address of a uninitialized variable that will be set when the function returns a value.
		 * @see MethodCall
		 * @see TypeTuple
		 */
		void(*MethodCaller)(const void*, void*, Storage::TypeTuple&, void*);

	public:

		/**
		 * Creates a FunctionInfo struct given the *function* address, name of the function and properties.
		 * @see FunctionProperties
		 */
		template <typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(*function)(TParameterTypes...), std::string_view name, FunctionProperties properties);

		/**
		 * Creates a FunctionInfo struct given the *method* address, name of the method and properties.
		 * @see FunctionProperties
		 */
		template <typename Class, typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(Class::*function)(TParameterTypes...), std::string_view name, FunctionProperties properties);

		/**
		 * Creates a FunctionInfo struct given the *const method* address, name of the method and properties.
		 * @see FunctionProperties
		 */
		template <typename Class, typename TReturnType, typename ... TParameterTypes>
		static FunctionInfo Create(TReturnType(Class::* function)(TParameterTypes...) const, std::string_view name, FunctionProperties properties);

		/**
		 * Tries casting the function address stored inside of the struct to a function type with the given ReturnT as return type, and ParameterTs as parameter types.
		 * @returns function pointer if successful or nullptr if not successful.
		 * @see TypesHash
		 */
		template <typename ReturnT, typename... ParameterTs>
		auto Cast() const->ReturnT(*)(ParameterTs...);

		/**
		 * Tries casting the method address stored inside of the struct to a method type with the given Class as the owning class, ReturnT as the return type, and ParameterTs as the parameter types.
		 * @returns method pointer if successful or nullptr if not successful.
		 * @see TypeHash
		 * @see OwningType
		 */
		template <typename Class, typename ReturnT, typename... ParameterTs>
		auto MethodCast() const->ReturnT(Class::*)(ParameterTs...);

		/**
		 * Call the function with the given parameters that are stored inside of the type tuple.
		 * @param parameters TypeTuple containing the data of the parameters.
		 * @param pReturnValue optional address of an uninitialized variable to be initialized when the function returns a value.
		 * @see TypeTuple
		 */
		inline void Call(Storage::TypeTuple& parameters, void* pReturnValue = nullptr) const;

		/**
		 * Call the member methods with the given parameters that are stored inside of the type tuple.
		 * @param subject pointer to the instance of the owning class
		 * @param parameters TypeTuple containing the data of the parameters.
		 * @param pReturnValue optional address of an uninitialized variable to be initialized when the function returns a value.
		 * @see TypeTuple
		 */
		inline void MemberCall(void* subject, Storage::TypeTuple& parameters, void* pReturnValue = nullptr) const;

		/**
		 * Check whether a property is set in this function/method
		 * @see FunctionProperties
		 */
		constexpr bool IsPropertySet(FunctionProperties property) const;

		/** Returns true if the function stored is a method*/
		constexpr bool IsMethod() const { return MethodCaller; }

		/**
		 * Check if the the given variables would be compatible with this function.
		 * First we check if the amount of types and the types are the same.
		 * Then we check if the variables and parameters are the same type.
		 * If the other variable is const but the function requires a reference or pointer that is not const, it is invalid.
		 * If the other variable is a pointer, but a reference is needed, then it is still valid.
		 * if the function parameter is a pointer, but the other variable is an instance, then it is still valid.
		 * @param otherVariables the variables that will be used to call the function.
		 */
		bool IsCompatible(std::span<const VariableId> otherVariables) const;
	};

	/**
	 * Class for identifying functions and methods.
	 * This class is easily copied and moved and maps to the actual FunctionInfo
	 * @see FunctionInfo
	 * @see GLAS_FUNCTION_ID
	 * @see GLAS_MEMBER_FUNCTION_ID
	 */
	class FunctionId final
	{
	public:
		constexpr FunctionId() = default;
		constexpr FunctionId(uint64_t functionHash) : m_FunctionHash{ functionHash } {}
	public:
		constexpr uint64_t GetId() const { return m_FunctionHash; }
		void SetId(uint64_t id) { m_FunctionHash = id; }

		/**
		 * Get the FunctionInfo associated with this function
		 * @see FunctionInfo
		 */
		const FunctionInfo* GetInfo() const;

		/**
		 * Cast the function address associated with this FunctionId to a function pointer with the given return type and parameter types.
		 * @see FunctionInfo::Cast
		 */
		template <typename ReturnType, typename... ParameterTypes>
		auto Cast() const->ReturnType(*)(ParameterTypes...);

		/**
		 * Cast the function address associated with this FunctionId to a method pointer of the given class with the given return type and parameter types.
		 * @see FunctionInfo::MethodCast
		 */
		template <typename Class, typename ReturnType, typename... ParameterTypes>
		auto MethodCast() const->ReturnType(Class::*)(ParameterTypes...);

		/**
		 * Creates a functionId from the given function.
		 * @param function function pointer.
		 * @param name name of the function.
		 * @returns generated function Id
		 * @see GLAS_MEMBER_FUNCTION_ID
		 * @see GLAS_FUNCTION_ID
		 */
		template <typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(*function)(ParameterTypes...), std::string_view name);

		/**
		 * Creates a functionId from the given method.
		 * @param function method pointer.
		 * @param name name of the method.
		 * @returns generated function Id
		 * @see GLAS_MEMBER_FUNCTION_ID
		 * @see GLAS_FUNCTION_ID
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(Class::* function)(ParameterTypes...), std::string_view name);

		/**
		 * Creates a functionId from the given const method.
		 * @param function method pointer.
		 * @param name name of the method.
		 * @returns generated function Id
		 * @see GLAS_MEMBER_FUNCTION_ID
		 * @see GLAS_FUNCTION_ID
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name);

		/**
		 * Get the functionId that is associated with the function address.
		 * Function must have been registered before using this function.
		 */
		template <typename ReturnType, typename ... ParameterTypes>
		static FunctionId GetFunctionId(ReturnType(*function)(ParameterTypes...));

		/**
		 * Get the functionId that is associated with the method address.
		 * Function must have been registered before using this function.
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId GetFunctionId(ReturnType(Class::* function)(ParameterTypes...));

		/**
		 * Get the functionId that is associated with the const method address.
		 * Function must have been registered before using this function.
		 */
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId GetFunctionId(ReturnType(Class::* function)(ParameterTypes...) const);

		/**
		 * Get the functionId that is associated with the function address.
		 * Function must have been registered before using this function.
		 */
		static FunctionId GetFunctionId(const void* functionAddress);

		/**
		 * Call the function associated with this function ID with the given parameters.
		 * @param parameters the values of the parameters that will used to call the function
		 * @param pReturnValue the address of an uninitialized variable that will be filled in case the function has a return value.
		 * @see TypeTuple
		 */
		inline void Call(Storage::TypeTuple& parameters, void* pReturnValue = nullptr) const;

		/**
		 * Call the function associated with this function ID with the given parameters.
		 * @param subject instance of the class that the method belongs to.
		 * @param parameters the values of the parameters that will used to call the function
		 * @param pReturnValue the address of an uninitialized variable that will be filled in case the function has a return value.
		 * @see TypeTuple
		 */
		inline void MemberCall(void* subject, Storage::TypeTuple& parameters, void* pReturnValue = nullptr) const;

	private:
		uint64_t m_FunctionHash{};
	};

	/**
	 * INHERITANCE REFLECTION
	 */

	/**
	 * Struct for describing the relation ship between a class and it parent class 
	 */
	struct BaseClassInfo
	{
		/**
		 * The Id of the parent class
		 */
		TypeId BaseId{};

		/**
		 * The offset in the child class that the parent class is located at.
		 * in case of multiple inheritance, the first inherited class is at offset 0, but the next data members of the other classes will be located further inside the class.
		 */
		size_t ClassOffset{};

		/**
		 * Creates a BaseClassInfo with the given Parent and Child classes
		 */
		template <typename Parent, typename Child>
		static constexpr BaseClassInfo Create();
	};

	/**
	* TypeInfo holds information about each type
	*/

	struct TypeInfo final
	{
		/** Name of the type. */
		std::string					Name{ };

		/**
		 * Size of the type gathered using sizeof().
		 * @see sizeof
		 */
		uint32_t					Size{ };

		/**
		 * Alignment of the type gathered using alignof()
		 * @see alignof
		 */
		uint32_t					Align{ };

		/**
		 * v-table pointer of the type in case the type is polymorphic
		 * @warning experimental
		 */
		const void*					VTable{ };

		/**
		 * Member variables that have been registered to this type.
		 * @see MemberInfo
		 * @see GlasAutoRegisterMember
		 * @see GLAS_MEMBER
		 */
		std::vector<MemberInfo>		Members{ };

		/**
		 * Member Functions that have been registered to this type.
		 * @see FunctionId
		 * @see GlasAutoRegisterFunction
		 * @see GLAS_FUNCTION
		 */
		std::vector<FunctionId>		MemberFunctions{ };

		/**
		 * Base classes of this type that have been registered.
		 * @see BaseClassInfo
		 * @see GlasAutoRegisterChildOnce
		 * @see GLAS_CHILD
		 */
		std::vector<BaseClassInfo>	BaseClasses{ };

		/**
		 * Child classes of this type that have been registered.
		 * @see TypeId
		 * @see GlasAutoRegisterChildOnce
		 * @see GLAS_CHILD
		 */
		std::vector<TypeId>			ChildClasses{ };

#ifdef GLAS_STORAGE
		/**
		 * Function pointer that constructs the type in place at the given address.
		 * @param 0 address for construction
		 */
		void (*Constructor)			(void*) { };

		/**
		 * Function pointer that constructs the type in place at the given address using the copy constructor.
		 * @param 0 address for construction
		 * @param 1 address of copyable type instance
		 * @see EnableCopyConstructor
		 */
		void (*CopyConstructor)		(void*, const void*) { };

		/**
		 * Function pointer that constructs the type in place at the given address using the move constructor.
		 * @param 0 address for construction
		 * @param 1 address of moveable type instance
		 * @see EnableMoveConstructor
		 */
		void (*MoveConstructor)		(void*, void*) { };

		/**
		 * Function pointer that destructs the type at the given address
		 * @param 0 address for destruction
		 */
		void (*Destructor)			(void*) { };

		/**
		 * Function pointer that swaps two instances of a type
		 * @param 0 address of type instance 1
		 * @param 1 address of type instance 2
		 */
		void (*Swap)				(void*, void*) { };
#endif // GLASS_STORAGE
#ifdef GLAS_SERIALIZATION_JSON
		using RapidJsonAllocator = RAPIDJSON_DEFAULT_ALLOCATOR;
		/**
		 * Function pointer that serializes the given type instance to a stream in json format
		 * @param 0 Rapid JSon Value
		 * @param 1 address of type instance that will be serialized
		 * @param 2 Rapid JSon Allocator
		 * @see Serialize
		 * @see GlasSerialize
		 */
		void (*JSonSerializer)		(rapidjson::Value&, const void*, RapidJsonAllocator&) { };

		/**
		 * Function pointer that initializes a type instance using a stream in json format
		 * @param 0 in stream containing data of type
		 * @param 1 address of type instance that will be initialized
		 * @see Deserialize
		 * @see GlasDeserialize
		 */
		void (*JSonDeserializer)	(rapidjson::Value&, void*) { };
#endif // GLAS_SERIALIZATION_JSON
#ifdef GLAS_SERIALIZATION_BINARY
		/**
		 * Function pointer that serialized the given type instance to a stream in binary format
		 * @param 0 out stream for serialization
		 * @param 1 address of type instance that will be serialized
		 * @see SerializeBinary
		 * @see GlasSerializeBinary
		 */
		void (*BinarySerializer)	(std::ostream&, const void*) { };

		/**
		 * Function pointer that initialized a type instance using a stream in binary format
		 * @param 0 in stream containing data of type
		 * @param 1 address of type instance that will be initialized
		 * @see DeserializeBinary
		 * @see GlasDeserializeBinary
		 */
		void (*BinaryDeserializer)	(std::istream&, void*) { };
#endif // GLAS_SERIALIZATION_BINARY
#ifdef GLAS_SERIALIZATION_YAML

		YAML::Node(*YamlSerializer)		(const void* data) { };

		void (*YamlDeserializer)	(const YAML::Node& node, void* data) { };

#endif // GLAS_SERIALIZATION_YAML

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

/**
 * HASH FUNCTIONS
 */

/** Hash function for TypeId to be used in std algorithms and unordered containers*/
template <>
struct std::hash<glas::TypeId>
{
	std::size_t operator()(const glas::TypeId& id) const noexcept
	{
		return static_cast<size_t>(id.GetId());
	}
};

/** Hash function for VariableId to be used in std algorithms and unordered containers*/
template <>
struct std::hash<glas::VariableId>
{
	std::size_t operator()(const glas::VariableId& id) const noexcept
	{
		return static_cast<size_t>(id.GetHash());
	}
};

/** Hash function for FunctionId to be used in std algorithms and unordered containers*/
template <>
struct std::hash<glas::FunctionId>
{
	std::size_t operator()(const glas::FunctionId& id) const noexcept
	{
		return static_cast<size_t>(id.GetId());
	}
};

namespace glas
{

	/**
	 * GLOBAL DATA
	 */

	/**
	 * This struct contains data about the reflection system that can be accessed anywhere in the program using glas::GetGlobalData().
	 * @see GetGlobalData
	 */
	struct GlobalData
	{
		GlobalData	()									= default;
		~GlobalData	()									= default;
		GlobalData	(const GlobalData&)					= delete;
		GlobalData	(GlobalData&&) noexcept				= delete;
		GlobalData& operator=(const GlobalData&)		= delete;
		GlobalData& operator=(GlobalData&&) noexcept	= delete;

		/**
		 * Map between TypeIds and their respective TypeInfos. Type must have been registered.
		 * @see TypeId::GetInfo
		 * @see GLAS_TYPE
		 */
		std::unordered_map<TypeId, TypeInfo>			TypeInfoMap{ };

		/**
		 * Map between the name of a type and its respective TypeId. Type must have been registered first
		 * @see GLAS_TYPE
		 */
		std::unordered_map<std::string, TypeId>			NameToTypeIdMap{ };

		/**
		 * Map between FunctionIds and their respective FunctionInfo. Function must have been registered.
		 * @see FunctionId::GetInfo
		 * @see GLAS_REGISTER_FUNCTION
		 * @see GLAS_MEMBER_REGISTER_FUNCTION
		 */
		std::unordered_map<FunctionId, FunctionInfo>	FunctionInfoMap{ };

		/**
		 * Map between a function name and its respective functionId. Function must have been registered.
		 * @see GLAS_REGISTER_FUNCTION
		 * @see GLAS_MEMBER_REGISTER_FUNCTION
		 * @warning experimental feature
		 */
		std::unordered_map<std::string, FunctionId>		NameToFunctionIdMap{ };

		/**
		 * Map between function addresses and their respective functionId. Function must have been registered.
		 * @see GLAS_REGISTER_FUNCTION
		 * @see GLAS_MEMBER_REGISTER_FUNCTION
		 */
		std::unordered_map<const void*, FunctionId>		FunctionAddressToIdMap{ };

		/**
		 * Map between v-table and TypeId in case the type is polymorphic. Type must have been registered.
		 * @warning experimental feature
		 */
		std::unordered_map<const void*, TypeId>			VTableMap{ };
	};

	/**
	 * Get the global data used in the reflection system.
	 * @see GlobalData
	 */
	inline GlobalData& GetGlobalData()
	{
		static GlobalData globalData{};
		return globalData;
	}

	/**
	 * HELPER FUNCTIONS
	 */

	/**
	 * Register a type into the reflection system.
	 * @see GlasAutoRegisterType
	 * @see GlasAutoRegisterTypeOnce
	 * @see GLAS_TYPE
	 */
	template <typename T>
	const TypeInfo& RegisterType();

	/**
	 * Register a member variable into the reflection system.
	 * @param fieldName the name of the member variable
	 * @param offset the offset of the member variable inside of the owning class
	 * @param properties custom set properties of the member variable
	 * @return reference to the generated MemberInfo instance
	 * @see GlasAutoRegisterMember
	 * @see GLAS_MEMBER
	 * @see MemberInfo
	 */
	template <typename Class, typename Field>
	const MemberInfo& RegisterField(std::string_view fieldName, uint32_t offset, MemberProperties properties = DefaultMemberProperties);

	/**
	 * Register a member variable into the reflection system.
	 * @param MemberId the identifier for the saved variable
	 * @param fieldName the name of the member variable
	 * @param offset the offset of the member variable inside of the owning class
	 * @param size the size of the member variable inside of the owning class
	 * @param align the alignment of the member variable inside of the owning class
	 * @param properties custom set properties of the member variable
	 * @return reference to the generated MemberInfo instance
	 * @see GlasAutoRegisterMember
	 * @see GLAS_MEMBER
	 * @see MemberInfo
	 */
	template <typename Class>
	const MemberInfo& RegisterField(VariableId MemberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align, MemberProperties properties = DefaultMemberProperties);

	/**
	 * Get the TypeInfo associated with the TypeId. asserts if the type has not been registered yet.
	 * @param id identifier of the type
	 * @return const reference to the TypeInfo
	 * @see TypeInfo
	 * @see TypeId
	 */
	const TypeInfo& GetTypeInfo(TypeId id);

	/**
	 * Get the TypeInfo associated with the given type T. assert if the type has not been registered yet.
	 * @return const reference to the TypeInfo
	 * @see TypeInfo
	 * @see TypeId
	 */
	template <typename T>
	const TypeInfo& GetTypeInfo();

	/**
	 * Get access to the unordered map containing all the typeIds and TypeInfos.
	 * @returns map containing all TypeInfos registered in the program.
	 */
	const std::unordered_map<TypeId, TypeInfo>& GetAllTypeInfo();

	/**
	 * Converts the given types to an std::array<VariableId>.
	 * @returns std::array<VariableId> containing the variableId representation of the types.
	 * @see VariableId
	 */
	template <typename... Types>
	constexpr std::array<VariableId, sizeof...(Types)> GetVariableArray();

	/**
	 * Converts the given types in the std::tuple to an std::array<VariableId>.
	 * @returns std::array<VariableId> containing the variableId representation of the types.
	 */
	template <typename Tuple>
	constexpr std::array<VariableId, std::tuple_size_v<Tuple>> GetVariableArrayTuple();

	/**
	 * Register function in the reflection system.
	 * @param function the function that has to be registered.
	 * @param name name of the function
	 * @param properties custom properties of the function.
	 * @see GlasAutoRegisterFunction
	 * @see GLAS_FUNCTION
	 * @see GLAS_FUNCTION_DEF
	 */
	template <typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name, FunctionProperties properties);

	/**
	 * Register member method in the reflection system
	 * @param function method that has to be registered.
	 * @param name name of the function
	 * @param properties custom properties of the method
	 * @see AutoRegisterMethod
	 * @see GLAS_METHOD
	 * @see GLAS_METHOD_DEF
	 */
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterMethodFunction(ReturnType(Class::* function)(ParameterTypes...), std::string_view name, FunctionProperties properties);

	/**
	 * Register const member method in the reflection system
	 * @param function method that has to be registered.
	 * @param name name of the function
	 * @param properties custom properties of the method
	 * @see AutoRegisterMethod
	 * @see GLAS_METHOD
	 * @see GLAS_METHOD_DEF
	 */
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterConstMethodFunction(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name, FunctionProperties properties);

	/**
	 * Calculates a uint64_t hash from the given Types.
	 * @returns hash of the types.
	 */
	template <typename... Types>
	constexpr uint64_t GetTypesHash();

	/**
	 * Calculates a hash for a Function. Takes the hash of the ReturnType and ParameterTypes and combines it with the hash of the function name.
	 * @returns hash of the function.
	 * @see GetTypeHash
	 */
	template <typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(*function)(ParameterTypes...), std::string_view name);

	/**
	 * Calculates a hash for a Function. Takes the hash of the ReturnType and ParameterTypes and combines it with the hash of the function name.
	 * @returns hash of the function.
	 * @see GetTypeHash
	 */
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::* function)(ParameterTypes...), std::string_view name);

	/**
	 * Calculates a hash for a Function. Takes the hash of the ReturnType and ParameterTypes and combines it with the hash of the function name.
	 * @returns hash of the function.
	 * @see GetTypeHash
	 */
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name);

	/**
	 * Get the offset of the Parent class inside of the Child class.
	 * In case of multiple inheritance the first parent class data sits at offset 0.
	 * the following parent classes data will start at the offset of the previous ones.
	 */
	template <typename Parent, typename Child>
	constexpr size_t GetClassOffset();

	/**
	 * Register the Parent-Child relation ship in the reflection system.
	 * @see GLAS_CHILD
	 */
	template <typename Parent, typename Child>
	void RegisterChild();

	template <typename T>
	const void* GetVTable(const T* instance) requires std::is_polymorphic_v<T>;

	template <typename T>
	TypeId GetTypeIDFromPolymorphic(const T* instance) requires std::is_polymorphic_v<T>;

	inline const void* VoidOffset(const void* data, size_t offset)
	{
		return static_cast<const uint8_t*>(data) + offset;
	}

	inline void* VoidOffset(void* data, size_t offset)
	{
		return static_cast<uint8_t*>(data) + offset;
	}

}

/** STATIC REGISTRATION*/

/**
 * Will register the Type whenever the variable is constructed.
 * When this struct is Initialized as a static or global variable, the type will be registered before the main function.
 * @see RegisterType
 * @see GlasAutoRegisterTypeOnce
 * @see GLAS_TYPE
 */
template <typename T>
struct GlasAutoRegisterType
{
	GlasAutoRegisterType()
	{
		glas::RegisterType<T>();
	}
};

/**
 * Will register the Type whenever an instance of GlasAutoRegisterTypeOnce<TYPE> exists.
 * @see RegisterType
 * @see GlasAutoRegisterType
 * @see GLAS_TYPE
 */
template <typename T>
struct GlasAutoRegisterTypeOnce
{
private:
	struct GlasAutoRegisterTypeOnce_Internal
	{
		GlasAutoRegisterTypeOnce_Internal()
		{
			glas::RegisterType<T>();
		}
	};
	inline static GlasAutoRegisterTypeOnce_Internal StaticRegisterType{};
};

/**
 * Will register the member variable when the type is constructed using the values inside of the constructor.
 * When this struct is initialized as a static or global variable, the member will be registered before the main function.
 * @see RegisterField
 * @see GLAS_MEMBER
 */
struct GlasAutoRegisterMember
{
	template <typename Class>
	GlasAutoRegisterMember(Class*, glas::VariableId memberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align, glas::MemberProperties properties = glas::DefaultMemberProperties)
	{
		glas::RegisterField<Class>(memberId, fieldName, offset, size, align, properties);
	}
};

/**
 * This section is for registering member variables.
 * It is more complex than the other registrations because this allows the registration of private member variables too.
 * We need to register the data in 2 steps.
 * 1. We register the "compile" type information (TypeId, offset, size, alignment)
 * 2. We register information that cannot be obtained by checking compile time data (Name and properties).
 */
namespace GlasMemberRegistration
{
	inline auto& GetRegisteredMembers()
	{
		static std::unordered_map<size_t, std::pair<glas::TypeId, uint32_t>> RegisteredMembers;
		return RegisteredMembers;
	}

	inline void RegisterMemberWithId(size_t id, std::pair<glas::TypeId, uint32_t> memberAccess)
	{
		GetRegisteredMembers().emplace(id, memberAccess);
	}

	inline glas::MemberInfo* GetMember(size_t id)
	{
		auto& member = GetRegisteredMembers()[id];
		return const_cast<glas::MemberInfo*>(member.first.GetMemberInfo(member.second));
	}

	inline void SetRuntimeProperties(size_t id, std::string_view name, glas::MemberProperties properties = glas::DefaultMemberProperties)
	{
		auto member = GetMember(id);
		assert(member);
		member->Name = name;
		member->Properties = properties;
	}
}

/**
 * Will register the compile time information about member variables whenever a proper instantiation of this type is created.
 * Usage: `template struct GlasRegisterMemberType<&CLASS::MEMBER, UNIQUE ID>`.
 * Putting this somewhere in the program will run the RegisterCompileTimeData function with the member pointer given in the template parameter.
 * We call this function because the `inline static const void* TypeAccessData` will construct itself using the result of the function.
 * Inside of the function we can infer the owning class and type of the member variable (only works with member variables and not functions).
 * This also works with with private and protected members because of the following rule: [temp.spec.general]/6 https://timsong-cpp.github.io/cppwp/n4868/temp.spec.general#6.
 * "The usual access checking rules do not apply to names in a declaration of an explicit instantiation or explicit specialization"
 */
template <auto Member, size_t ID>
struct GlasRegisterMemberType
{
	template <typename Class, typename T>
	static void* RegisterCompileTimeData(T Class::* member)
	{
		const glas::MemberInfo& memberInfo = RegisterField<Class>(
			glas::VariableId::Create<T>(),
			"",
			static_cast<uint32_t>(reinterpret_cast<size_t>(&(*static_cast<Class*>(nullptr).*member))),
			sizeof(decltype(member)),
			alignof(decltype(member)),
			glas::DefaultMemberProperties
		);

		GlasMemberRegistration::RegisterMemberWithId(ID, std::pair<glas::TypeId, uint32_t>(glas::TypeId::Create<Class>(), memberInfo.Offset));

		return nullptr;
	}

	inline static const void* TypeAccessData = RegisterCompileTimeData(Member);
};

struct GlasAutoMemberVariableDataSetter
{
	GlasAutoMemberVariableDataSetter(size_t id, std::string_view name, glas::MemberProperties properties = glas::DefaultMemberProperties)
	{
		GlasMemberRegistration::SetRuntimeProperties(id, name, properties);
	}
};

/**
 * Will register the function when an instance of the type is constructed.
 * When this struct is initialized as a static or global variable, the function will be registered before the main function.
 * @see RegisterFunction
 * @see GLAS_FUNCTION
 */
struct GlasAutoRegisterFunction
{
	template <typename ReturnType, typename ... ParameterTypes>
	GlasAutoRegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name, glas::FunctionProperties properties = glas::DefaultFunctionProperties)
	{
		glas::RegisterFunction(function, name, properties);
	}
};

/**
 * Will register the member method when an instance of the type is constructed.
 * When this struct is initialized as a static or global variable, the method will be registered before the main function.
 * @see RegisterMethodFunction
 * @see RegisterConstMethodFunction
 * @see GLAS_MEMBER
 */
struct GlasAutoRegisterMemberFunction
{
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	GlasAutoRegisterMemberFunction(ReturnType(Class::*function)(ParameterTypes...), std::string_view name, glas::FunctionProperties properties = glas::DefaultFunctionProperties)
	{
		glas::RegisterMethodFunction(function, name, properties);
	}
	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	GlasAutoRegisterMemberFunction(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name, glas::FunctionProperties properties = glas::DefaultFunctionProperties)
	{
		glas::RegisterConstMethodFunction(function, name, properties);
	}
};

/**
 * Will register the parent-child relationship in the reflection system when a instance of the type is constructed.
 * When this struct is initialized as a static or global variable, the relationship will be registered before the main function.
 * @see RegisterChild
 * @see GlasAutoRegisterChildOnce
 * @see GLAS_CHILD
 */
template <typename Parent, typename Child>
struct GlasAutoRegisterChild
{
	GlasAutoRegisterChild()
	{
		glas::RegisterChild<Parent, Child>();
	}
};

/**
 * Will register the parent-child relationship in the reflection system whenever an instance of this class exists.
 * @see GlasAutoRegisterChild
 * @see RegisterChild
 * @see GLAS_CHILD
 */
template <typename Parent, typename Child>
struct GlasAutoRegisterChildOnce
{
private:
	struct GlasAutoRegisterChildOnce_Internal
	{
		GlasAutoRegisterChildOnce_Internal()
		{
			glas::RegisterChild<Parent, Child>();
		}
	};
	inline static GlasAutoRegisterChildOnce_Internal StaticRegisterType{};
};


/**
 * Internal Macros
 */

/** Concatenates two strings together inside a Macro.*/
#define _GLAS_CONCAT_(a,b) a ## b

/** Creates an inline static variable to register the Given Type.*/
#define _GLAS_TYPE_INTERNAL(TYPE, ID) inline static GlasAutoRegisterType<TYPE> _GLAS_CONCAT_(RegisterType_, ID) {};

/** Creates an inline static variable that registers a member variable.*/
#define _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, ID) inline static GlasAutoRegisterMember _GLAS_CONCAT_(RegisterMember_, ID) { static_cast<TYPE*>(nullptr), glas::VariableId::Create<decltype(TYPE::MEMBER)>(), #MEMBER, offsetof(TYPE, MEMBER), sizeof(decltype(TYPE::MEMBER)), alignof(decltype(TYPE::MEMBER)), PROPERTIES};

/** Creates an inline static variable that registers a private member variable*/
#define _GLAS_PRIVATE_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, ID) template struct GlasRegisterMemberType<&TYPE::MEMBER, ID>; \
inline static GlasAutoMemberVariableDataSetter _GLAS_CONCAT_(RegisterMember_, ID){ID, #MEMBER, PROPERTIES};

/** Creates an inline static variable that registers a function.*/
#define _GLAS_FUNCTION_INTERNAL(FUNCTION, ID, PROPS) inline static GlasAutoRegisterFunction _GLAS_CONCAT_(RegisterFunction_, ID) {FUNCTION, #FUNCTION, PROPS};

/** Creates an inline static variable that registers a member function.*/
#define _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, ID) inline static GlasAutoRegisterMemberFunction _GLAS_CONCAT_(RegisterMemberFunction_, ID) {&CLASS::FUNCTION, #FUNCTION, PROPS};

/** Creates an inline static variable that registers a parent-child relationship.*/
#define _GLAS_CHILD_INTERNAL(BASE, CHILD, ID) inline static GlasAutoRegisterChildOnce<BASE, CHILD> _GLAS_CONCAT_(RegisterChild_, ID) {};



/**
 * Public Macros
 */

/** Register a type.*/
#define GLAS_TYPE(TYPE) _GLAS_TYPE_INTERNAL(TYPE, __COUNTER__)

/** Register a member variable with properties.*/
#define GLAS_MEMBER_PROP(TYPE, MEMBER, PROPERTIES)  _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, __COUNTER__)
/** Register a member variable with default properties.*/
#define GLAS_MEMBER(TYPE, MEMBER) _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, glas::DefaultMemberProperties, __COUNTER__)

/** Register a private member variable with properties. This can only be used inside of the global namespace (not withing a namespace).*/
#define GLAS_PRIVATE_MEMBER_PROP(TYPE, MEMBER, PROPERTIES) _GLAS_PRIVATE_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, __COUNTER__)
/** Register a private member variable with default properties. This can only be used inside of the global namespace (not withing a namespace).*/
#define GLAS_PRIVATE_MEMBER(TYPE, MEMBER) _GLAS_PRIVATE_MEMBER_INTERNAL(TYPE, MEMBER, glas::DefaultMemberProperties, __COUNTER__)

/** Get the function ID from the given function.*/
#define GLAS_FUNCTION_ID(FUNCTION) glas::FunctionId::Create(FUNCTION, #FUNCTION)
/** Get the function ID from the given method.*/
#define GLAS_MEMBER_FUNCTION_ID(CLASS, FUNCTION) glas::FunctionId::Create(&CLASS::FUNCTION, #FUNCTION)

/** Register a function with properties.*/
#define GLAS_FUNCTION_PROP(FUNCTION, PROPS) _GLAS_FUNCTION_INTERNAL(FUNCTION, __COUNTER__, PROPS);
/** Register a function with default properties.*/
#define GLAS_FUNCTION(FUNCTION) _GLAS_FUNCTION_INTERNAL(FUNCTION, __COUNTER__, glas::DefaultFunctionProperties);

/** Register a member function with properties.*/
#define GLAS_MEMBER_FUNCTION_PROP(CLASS, FUNCTION, PROPS) _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, __COUNTER__);
/** Register a member function with default properties.*/
#define GLAS_MEMBER_FUNCTION(CLASS, FUNCTION) _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, glas::DefaultFunctionProperties, __COUNTER__);

/** Register a parent child relation ship.*/
#define GLAS_CHILD(BASE, CHILD) _GLAS_CHILD_INTERNAL(BASE, CHILD, __COUNTER__)