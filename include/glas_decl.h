#pragma once

#include <span>
#include <array>
#include <tuple>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <functional>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "glas_properties.h"

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
		VariableId			VariableId	{ }; /**< Type Information about the member variable*/
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
		 * @see MethodCall
		 * @see TypeTuple
		 */
		void(*FunctionCaller)(const void*, Storage::TypeTuple&, void*);

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

		/** Returns true if the function stored is a property*/
		constexpr bool IsMethod() const { return OwningType.IsValid(); }
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
		std::unordered_map<TypeId, TypeInfo>			TypeInfoMap				{ };

		/**
		 * Map between the name of a type and its respective TypeId. Type must have been registered first
		 * @see GLAS_TYPE
		 */
		std::unordered_map<std::string, TypeId>			NameToTypeIdMap			{ };

		/**
		 * Map between FunctionIds and their respective FunctionInfo. Function must have been registered.
		 * @see FunctionId::GetInfo
		 * @see GLAS_REGISTER_FUNCTION
		 * @see GLAS_MEMBER_REGISTER_FUNCTION
		 */
		std::unordered_map<FunctionId, FunctionInfo>	FunctionInfoMap			{ };

		/**
		 * Map between a function name and its respective functionId. Function must have been registered.
		 * @see GLAS_REGISTER_FUNCTION
		 * @see GLAS_MEMBER_REGISTER_FUNCTION
		 * @warning experimental feature
		 */
		std::unordered_map<std::string, FunctionId>		NameToFunctionIdMap		{ };

		/**
		 * Map between function addresses and their respective functionId. Function must have been registered.
		 * @see GLAS_REGISTER_FUNCTION
		 * @see GLAS_MEMBER_REGISTER_FUNCTION
		 */
		std::unordered_map<const void*, FunctionId>		FunctionAddressToIdMap	{ };

		/**
		 * Map between v-table and TypeId in case the type is polymorphic. Type must have been registered.
		 * @warning experimental feature
		 */
		std::unordered_map<const void*, TypeId>			VTableMap				{ };
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
	 * @see AutoRegisterType
	 * @see AutoRegisterTypeOnce
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
	 * @see AutoRegisterMember
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
	 * @see AutoRegisterMember
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

	template <typename... Types>
	constexpr std::array<VariableId, sizeof...(Types)> GetVariableArray();

	template <typename Tuple>
	constexpr std::array<VariableId, std::tuple_size_v<Tuple>> GetVariableArrayTuple();

	template <typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name, FunctionProperties properties);

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterMethodFunction(ReturnType(Class::* function)(ParameterTypes...), std::string_view name, FunctionProperties properties);

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterConstMethodFunction(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name, FunctionProperties properties);

	template <typename... Types>
	constexpr uint64_t GetTypesHash();

	template <typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(*function)(ParameterTypes...), std::string_view name);

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::*function)(ParameterTypes...), std::string_view name);

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name);

	template <typename Parent, typename Child>
	constexpr size_t GetClassOffset();

	template <typename Parent, typename Child>
	constexpr void RegisterChild();

	/** STATIC REGISTRATION*/

	template <typename T>
	struct AutoRegisterType
	{
		AutoRegisterType()
		{
			RegisterType<T>();
		}
	};

	template <typename T>
	struct AutoRegisterTypeOnce
	{
	private:
		struct AutoRegisterTypeOnce_Internal
		{
			AutoRegisterTypeOnce_Internal()
			{
				RegisterType<T>();
			}
		};
		inline static AutoRegisterTypeOnce_Internal StaticRegisterType{};
	};

	struct AutoRegisterMember
	{
		template <typename Class>
		AutoRegisterMember(Class*, VariableId memberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align, MemberProperties properties = DefaultMemberProperties)
		{
			RegisterField<Class>(memberId, fieldName, offset, size, align, properties);
		}
	};

	struct AutoRegisterFunction
	{
		template <typename ReturnType, typename ... ParameterTypes>
		AutoRegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			RegisterFunction(function, name, properties);
		}
	};

	struct AutoRegisterMemberFunction
	{
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		AutoRegisterMemberFunction(ReturnType(Class::*function)(ParameterTypes...), std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			RegisterMethodFunction(function, name, properties);
		}
		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		AutoRegisterMemberFunction(ReturnType(Class::* function)(ParameterTypes...) const, std::string_view name, FunctionProperties properties = DefaultFunctionProperties)
		{
			RegisterConstMethodFunction(function, name, properties);
		}
	};

	template <typename Parent, typename Child>
	struct AutoRegisterChild
	{
		AutoRegisterChild()
		{
			RegisterChild<Parent, Child>();
		}
	};

	template <typename Parent, typename Child>
	struct AutoRegisterChildOnce
	{
	private:
		struct AutoRegisterChildOnce_Internal
		{
			AutoRegisterChildOnce_Internal()
			{
				RegisterChild<Parent, Child>();
			}
		};
		inline static AutoRegisterChildOnce_Internal StaticRegisterType{};
	};
}

#define _GLAS_CONCAT_(a,b) a ## b

#define _GLAS_TYPE_INTERNAL(TYPE, ID) inline static glas::AutoRegisterType<TYPE> _GLAS_CONCAT_(RegisterType_, ID) {};
#define GLAS_TYPE(TYPE) _GLAS_TYPE_INTERNAL(TYPE, __LINE__)

#define _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, ID) inline static glas::AutoRegisterMember _GLAS_CONCAT_(RegisterMember_, ID) { static_cast<TYPE*>(nullptr), glas::VariableId::Create<decltype(TYPE::MEMBER)>(), #MEMBER, offsetof(TYPE, MEMBER), sizeof(decltype(TYPE::MEMBER)), alignof(decltype(TYPE::MEMBER)), PROPERTIES};
#define GLAS_MEMBER(TYPE, MEMBER, PROPERTIES)  _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, PROPERTIES, __LINE__)
#define GLAS_MEMBER_DEF(TYPE, MEMBER)  _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, glas::DefaultMemberProperties, __LINE__)

#define GLAS_FUNCTION_ID(FUNCTION) glas::FunctionId::Create(FUNCTION, #FUNCTION)
#define GLAS_MEMBER_FUNCTION_ID(CLASS, FUNCTION) glas::FunctionId::Create(&CLASS::FUNCTION, #FUNCTION)

#define _GLAS_FUNCTION_INTERNAL(FUNCTION, ID, PROPS) inline static glas::AutoRegisterFunction _GLAS_CONCAT_(RegisterFunction_, ID) {FUNCTION, #FUNCTION, PROPS};
#define GLAS_FUNCTION(FUNCTION, PROPS) _GLAS_FUNCTION_INTERNAL(FUNCTION, __LINE__, PROPS);
#define GLAS_FUNCTION_DEF(FUNCTION) _GLAS_FUNCTION_INTERNAL(FUNCTION, __LINE__, glas::DefaultFunctionProperties);

#define _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, ID) inline static glas::AutoRegisterMemberFunction _GLAS_CONCAT_(RegisterMemberFunction_, ID) {&CLASS::FUNCTION, #FUNCTION, PROPS};
#define GLAS_MEMBER_FUNCTION(CLASS, FUNCTION, PROPS) _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, PROPS, __LINE__);
#define GLAS_MEMBER_FUNCTION_DEF(CLASS, FUNCTION) _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, glas::DefaultFunctionProperties, __LINE__);

#define _GLAS_CHILD_INTERNAL(BASE, CHILD, ID) inline static glas::AutoRegisterChildOnce<BASE, CHILD> _GLAS_CONCAT_(RegisterChild_, ID) {};
#define GLAS_CHILD(BASE, CHILD) _GLAS_CHILD_INTERNAL(BASE, CHILD, __LINE__)