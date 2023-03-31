#pragma once

#include <string_view>
#include <cstdint>

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

	constexpr uint64_t hash(std::string_view str)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (char c : str)
		{
			hash_value ^= static_cast<std::uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}

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
		using Type = remove_all_pointers<T>::Type;
	};

	template <typename T>
	using remove_all_pointers_t = remove_all_pointers<T>::Type;

	template <typename T> struct strip_type
	{
		using Type = std::remove_cvref_t<remove_all_pointers_t<std::remove_reference_t<std::remove_all_extents_t<T>>>>;
	};

	template <typename T>
	using strip_type_t = strip_type<T>::Type;

	struct TypeInfo;
	class TypeId;
	class VariableId;
	struct MemberInfo;

	class TypeId final
	{
	public:
		constexpr TypeId() = default;
		constexpr TypeId(uint64_t id) : ID{ id } {};

	public:
		template <typename T>
		static constexpr TypeId Create()
		{
			AutoRegisterTypeOnce<T>();
			return TypeId(TypeHash<strip_type_t<T>>());
		}

	public:
		constexpr void			SetTypeId			(uint64_t typeId)	{ ID = typeId; }
		constexpr uint64_t		GetId				()	const			{ return ID; }

	private:
		uint64_t ID{};
	};

	class VariableId final
	{
	private:

		static constexpr uint32_t ConstFlag				= 1 << 0;
		static constexpr uint32_t ReferenceFlag			= 1 << 1;
		static constexpr uint32_t VolatileFlag			= 1 << 2;
		static constexpr uint32_t RValReferenceFlag		= 1 << 3;

	public:

		constexpr explicit VariableId(TypeId id) : Type{ id } {};
		constexpr VariableId() = default;

		template <typename T>
		static constexpr VariableId Create();

	public:

		constexpr TypeId GetTypeId						() const			{ return Type; }
		constexpr void SetTypeId						(TypeId id)			{ Type = id; }

		constexpr void SetConstFlag						()					{ TraitFlags |= ConstFlag; }
		constexpr void SetReferenceFlag					()					{ TraitFlags |= ReferenceFlag; }
		constexpr void SetVolatileFlag					()					{ TraitFlags |= VolatileFlag; }
		constexpr void SetRValReferenceFlag				()					{ TraitFlags |= RValReferenceFlag; }

		constexpr void RemoveConstFlag					()					{ TraitFlags &= ~ConstFlag; }
		constexpr void RemoveReferenceFlag				()					{ TraitFlags &= ~ReferenceFlag; }
		constexpr void RemoveVolatileFlag				()					{ TraitFlags &= ~VolatileFlag; }
		constexpr void RemoveRValReferenceFlag			()					{ TraitFlags &= ~RValReferenceFlag; }

		constexpr void SetPointerAmount					(uint16_t amount)	{ PointerAmount = amount; }
		constexpr uint32_t GetPointerAmount				() const			{ return PointerAmount; }

		constexpr void SetArraySize						(uint32_t Size)		{ ArraySize = Size; }
		constexpr uint32_t GetArraySize					() const			{ return ArraySize; }

		constexpr bool IsConst							() const			{ return TraitFlags & ConstFlag; }
		constexpr bool IsReference						() const			{ return TraitFlags & ReferenceFlag; }
		constexpr bool IsVolatile						() const			{ return TraitFlags & VolatileFlag; }
		constexpr bool IsRValReference					() const			{ return TraitFlags & RValReferenceFlag; }
		constexpr bool IsPointer						() const			{ return PointerAmount; }
		constexpr bool IsArray							() const			{ return ArraySize == 1; }
		constexpr bool IsRefOrPointer					() const			{ return IsPointer() || IsReference() || IsRValReference(); }

		constexpr uint64_t	GetHash						() const			{ return Type.GetId() ^ ArraySize ^ (static_cast<uint64_t>(PointerAmount) << 32) ^ (static_cast<uint64_t>(TraitFlags) << 40); }

		//constexpr uint32_t GetSize						() const			{ return IsRefOrPointer() ? sizeof(void*) : (GetTypeId().GetTypeSize() * GetArraySize()); }
		//constexpr uint32_t GetAlign						() const			{ return IsRefOrPointer() ? alignof(void*) : GetTypeId().GetTypeAlignment(); }

	private:

		TypeId		Type			{ };	// The underlying type id
		uint32_t	ArraySize		{ };	// if the variable is a fixed sized array, the size will be contained in this. else it will be 1
		uint16_t	PointerAmount	{ };	// The amount of pointers that are attached to the Type
		uint8_t		TraitFlags		{ };	// Other flags (const, volatile, reference, RValReference)
	};

	struct MemberInfo final
	{
		std::string_view	Name		{ };
		VariableId			VariableId	{ };
		uint32_t			Offset		{ };
		uint32_t			Size		{ };
		uint32_t			Align		{ };

		constexpr bool operator<(const MemberInfo& rhs) const
		{
			return Offset < rhs.Offset;
		}
	};

	template <typename T>
	const TypeInfo& RegisterType();

	template <typename Class, typename Field>
	static const MemberInfo& RegisterField(const std::string_view fieldName, uint32_t Offset);

	template <typename Field>
	static const MemberInfo& RegisterField(TypeId classId, const std::string_view fieldName, uint32_t Offset);

	template <typename Class>
	static const MemberInfo& RegisterField(TypeId classId, VariableId MemberId, const std::string_view fieldName, uint32_t Offset, uint32_t Size, uint32_t Align);

	const TypeInfo& GetTypeInfo(TypeId id);

	template <typename T>
	const TypeInfo& GetTypeInfo();

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
		AutoRegisterMember(TypeId classId, VariableId memberId, std::string_view fieldName, uint32_t offset, uint32_t size, uint32_t align)
		{
			RegisterField<Class>(classId, memberId, fieldName, offset, size, align);
		}
	};
}

#define _GLAS_TYPE_INTERNAL(TYPE, VARNAME) glas::AutoRegisterType<TYPE> VARNAME##TYPE {};
#define GLAS_TYPE(TYPE) _GLAS_TYPE_INTERNAL(TYPE, RegisterType_)

#define GLAS_MEMBER(TYPE, MEMBER) inline static glas::AutoRegisterMember<TYPE> TYPE##MEMBER {glas::TypeId::Create<TYPE>(), glas::VariableId::Create<decltype(TYPE::MEMBER)>(), #MEMBER, offsetof(TYPE, MEMBER), sizeof(decltype(TYPE::MEMBER)), alignof(decltype(TYPE::MEMBER))};

