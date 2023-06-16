#pragma once

#include <string_view>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <span>

namespace glas
{
	//https://stackoverflow.com/questions/35941045/can-i-obtain-c-type-names-in-a-constexpr-way
	//asnwer by einpoklum

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

	constexpr uint64_t hash(std::string_view str);

	constexpr uint64_t hash(std::span<const uint64_t> span);

	template <typename Type>
	constexpr uint64_t TypeHash()
	{
		return hash(TypeName<Type>());
	}

	template <typename T>
	constexpr uint32_t CountPointers(uint32_t counter = 0)
	{
		if constexpr (std::is_pointer_v<T>)
			return CountPointers<std::remove_pointer_t<T>>(++counter);
		else
			return counter;
	}

	//https://stackoverflow.com/questions/9851594/standard-c11-way-to-remove-all-pointers-of-a-type
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
	 * @see Create()
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
		 * Static templated constexpr function that creates a m_TypeId from the given Type T.
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

		const MemberInfo*		GetMemberInfo(size_t offset)		const;

	private:
		uint64_t m_ID{};
	};

	/**
	 * VARIABLE REFLECTION
	 */

	class VariableId final
	{
	private:

		static constexpr uint32_t ConstFlag			= 1 << 0;
		static constexpr uint32_t ReferenceFlag		= 1 << 1;
		static constexpr uint32_t VolatileFlag		= 1 << 2;
		static constexpr uint32_t RValReferenceFlag = 1 << 3;

	public:

		constexpr explicit VariableId(TypeId id) : m_Type{ id } {}
		constexpr VariableId() = default;

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

		constexpr uint64_t	GetHash					()				const	{ return m_Type.GetId() ^ m_ArraySize ^ (static_cast<uint64_t>(m_PointerAmount) << 32) ^ (static_cast<uint64_t>(m_TraitFlags) << 40); }

		constexpr uint32_t	GetSize					()				const;
		constexpr uint32_t	GetAlign				()				const;

		friend constexpr bool operator==(const VariableId& lhs, const VariableId& rhs);
		friend std::ostream& operator<<(std::ostream& lhs, const VariableId& rhs);
		friend std::istream& operator>>(std::istream& lhs, const VariableId& rhs);

		std::string			ToString				()				const;

	private:

		TypeId		m_Type			{ };	// The underlying type id
		uint32_t	m_ArraySize		{ };	// if the variable is a fixed sized array, the size will be contained in this. else it will be 1
		uint16_t	m_PointerAmount	{ };	// The amount of pointers that are attached to the Type
		uint8_t		m_TraitFlags	{ };	// Other flags (const, volatile, reference, RValReference)
	};

	/**
	 * MEMBER VARIABLE REFLECTION
	 */

	struct MemberInfo final
	{
		std::string_view	Name		{ };
		VariableId			VariableId	{ };
		uint32_t			Offset		{ };
		uint32_t			Size		{ };
		uint32_t			Align		{ };

		constexpr bool operator<(const MemberInfo& rhs) const { return Offset < rhs.Offset; }
	};

	/**
	 * FUNCTION REFLECTION
	 */

	struct FunctionInfo final
	{
		const void*				FunctionAddress{ };
		VariableId				ReturnType{ };
		std::string_view		Name{ };
		uint64_t				TypesHash{ };
		std::vector<VariableId>	ParameterTypes{ };

		void(*FunctionCaller)(const void* address, Storage::TypeTuple& typeTuple, void* returnAddress);

		template <typename ReturnT, typename... ParameterTs>
		auto Cast() const->ReturnT(*)(ParameterTs...);

		void Call(Storage::TypeTuple& typeTuple, void* pReturnValue = nullptr) const;
	};

	class FunctionId final
	{
		constexpr FunctionId() = default;
		constexpr FunctionId(uint64_t functionHash) : m_FunctionHash{ functionHash } {}
	public:
		constexpr uint64_t GetId() const { return m_FunctionHash; }
		const FunctionInfo& GetInfo() const;

		template <typename ReturnType, typename... ParameterTypes>
		auto Cast() const->ReturnType(*)(ParameterTypes...);

		template <typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(*function)(ParameterTypes...), std::string_view name);

		template <typename Class, typename ReturnType, typename ... ParameterTypes>
		static FunctionId Create(ReturnType(Class::* function)(ParameterTypes...), std::string_view name);

		void Call(Storage::TypeTuple& typeTuple, void* pReturnValue = nullptr) const;

	private:
		uint64_t m_FunctionHash{};
	};

	/**
	 * INHERITANCE REFLECTION
	 */

	struct BaseClassInfo
	{
		TypeId BaseId{};
		size_t ClassOffset{};

		template <typename Parent, typename Child>
		static constexpr BaseClassInfo Create();
	};
}

/**
 * HASH FUNCTIONS
 */

template <>
struct std::hash<glas::TypeId>
{
	std::size_t operator()(const glas::TypeId& id) const noexcept
	{
		return static_cast<size_t>(id.GetId());
	}
};

template <>
struct std::hash<glas::VariableId>
{
	std::size_t operator()(const glas::VariableId& id) const noexcept
	{
		return static_cast<size_t>(id.GetHash());
	}
};

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

	struct GlobalData
	{
		std::unordered_map<FunctionId, FunctionInfo> FunctionInfoMap{};
		std::unordered_map<std::string_view, FunctionId> NameToFunctionIdMap{};
		std::unordered_map<std::string_view, TypeId> NameToTypeIdMap{};
		std::unordered_map<void*, TypeId> VTableMap{};
	};

	inline GlobalData& GetGlobalData()
	{
		static GlobalData globalData{};
		return globalData;
	}

	/**
	 * HELPER FUNCTIONS
	 */

	template <typename T>
	const TypeInfo& RegisterType();

	template <typename Class, typename Field>
	const MemberInfo& RegisterField(std::string_view fieldName, uint32_t Offset);

	template <typename Class>
	const MemberInfo& RegisterField(VariableId MemberId, std::string_view fieldName, uint32_t Offset, uint32_t Size, uint32_t Align);

	const TypeInfo& GetTypeInfo(TypeId id);

	template <typename T>
	const TypeInfo& GetTypeInfo();

	const std::unordered_map<TypeId, TypeInfo>& GetAllTypeInfo();

	template <typename... Types>
	constexpr std::array<VariableId, sizeof...(Types)> GetVariableArray();

	template <typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name);

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterMethodFunction(ReturnType(Class::* function)(ParameterTypes...), std::string_view name);

	template <typename... Types>
	constexpr uint64_t GetTypesHash();

	template <typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(*function)(ParameterTypes...), std::string_view name);

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(Class::*function)(ParameterTypes...), std::string_view name);

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

	template <typename Class>
	struct AutoRegisterMember
	{
		AutoRegisterMember(VariableId memberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align)
		{
			RegisterField<Class>(memberId, fieldName, offset, size, align);
		}
	};

	template <typename ReturnType, typename ... ParameterTypes>
	struct AutoRegisterFunction
	{
		AutoRegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name)
		{
			RegisterFunction(function, name);
		}
	};

	template <typename Class, typename ReturnType, typename ... ParameterTypes>
	struct AutoRegisterMemberFunction
	{
		AutoRegisterMemberFunction(ReturnType(Class::*function)(ParameterTypes...), std::string_view name)
		{
			RegisterMethodFunction(function, name);
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

#define _CONCAT_(a,b) a ## b

#define _GLAS_TYPE_INTERNAL(TYPE, ID) glas::AutoRegisterType<TYPE> _CONCAT_(RegisterType_, ID) {};
#define GLAS_TYPE(TYPE) _GLAS_TYPE_INTERNAL(TYPE, __LINE__)

#define _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, ID) inline static glas::AutoRegisterMember<TYPE> _CONCAT_(RegisterMember_, ID) {glas::VariableId::Create<decltype(TYPE::MEMBER)>(), #MEMBER, offsetof(TYPE, MEMBER), sizeof(decltype(TYPE::MEMBER)), alignof(decltype(TYPE::MEMBER))};
#define GLAS_MEMBER(TYPE, MEMBER)  _GLAS_MEMBER_INTERNAL(TYPE, MEMBER, __LINE__)

#define _GLAS_FUNCTION_INTERNAL(FUNCTION, ID) inline static glas::AutoRegisterFunction _CONCAT_(RegisterFunction_, ID) {FUNCTION, #FUNCTION};
#define GLAS_FUNCTION(FUNCTION) _GLAS_FUNCTION_INTERNAL(FUNCTION, __LINE__);

#define _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, ID) inline static glas::AutoRegisterMemberFunction _CONCAT_(RegisterMemberFunction_, ID) {&CLASS::FUNCTION, #FUNCTION};
#define GLAS_MEMBER_FUNCTION(CLASS, FUNCTION) _GLAS_MEMBER_FUNCTION_INTERNAL(CLASS, FUNCTION, __LINE__);

#define _GLAS_CHILD_INTERNAL(BASE, CHILD, ID) inline static glas::AutoRegisterChildOnce<BASE, CHILD> _CONCAT_(RegisterChild_, ID) {};
#define GLAS_CHILD(BASE, CHILD) _GLAS_CHILD_INTERNAL(BASE, CHILD, __LINE__)

