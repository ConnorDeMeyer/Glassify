#pragma once

#include "glas_decl.h"
#include "glas_config.h"

#include <cassert>

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

namespace glas
{

	constexpr bool operator==(TypeId lhs, TypeId rhs)
	{
		return lhs.GetId() == rhs.GetId();
	}

	template <typename T>
	static constexpr VariableId VariableId::Create()
	{
		using Type_RemovedExtents	= std::remove_all_extents_t<T>;
		using Type_RemovedRefs		= std::remove_reference_t<Type_RemovedExtents>;
		using Type_RemovedPtrs		= remove_all_pointers_t<Type_RemovedRefs>;

		using StrippedType			= std::remove_cvref_t<Type_RemovedPtrs>;
		
		AutoRegisterTypeOnce<StrippedType> TypeRegister{};

		constexpr bool IsRef		{ std::is_reference_v<T> };
		constexpr bool IsRValRef	{ std::is_rvalue_reference_v<T> };
		constexpr bool IsConst		{ std::is_const_v<Type_RemovedPtrs> };
		constexpr bool IsVolatile	{ std::is_volatile_v<Type_RemovedPtrs> };

		constexpr uint32_t PointerAmount{ CountPointers<Type_RemovedRefs>() };

		auto variable = VariableId(TypeId::Create<StrippedType>());

		if constexpr (IsConst)		variable.SetConstFlag();
		if constexpr (IsVolatile)	variable.SetVolatileFlag();
		if constexpr (IsRef)		variable.SetReferenceFlag();
		if constexpr (IsRValRef)	variable.SetRValReferenceFlag();

		variable.SetPointerAmount(PointerAmount);

		if constexpr (!std::is_same_v<void, Type_RemovedExtents>)
		{
			constexpr uint32_t ArraySize{ sizeof(T) / sizeof(Type_RemovedExtents) };
			variable.SetArraySize(ArraySize);
		}
		else
		{
			variable.SetArraySize(1);
		}

		return variable;
	}

	std::unordered_map<TypeId, TypeInfo>& GetTypeInfoMap()
	{
		static std::unordered_map<TypeId, TypeInfo> TypeInfoMap;
		return TypeInfoMap;
	}

	const std::unordered_map<TypeId, TypeInfo>& GetAllTypeInfo()
	{
		return GetTypeInfoMap();
	}

	template <typename T>
	const TypeInfo& RegisterType()
	{
		auto& typeInfoMap = GetTypeInfoMap();
		
		constexpr TypeId hash = TypeId::Create<T>();

		auto it = typeInfoMap.find(hash);
		if (it == typeInfoMap.end())
		{
			return typeInfoMap.emplace(
				hash,
				TypeInfo::Create<T>()
			).first->second;
		}
		return it->second;

	}

	const TypeInfo& glas::GetTypeInfo(TypeId id)
	{
		assert(GetTypeInfoMap().contains(id));
		return GetTypeInfoMap()[id];
	}

	template<typename T>
	const TypeInfo& GetTypeInfo()
	{
		return GetTypeInfo(TypeId::Create<T>());
	}

	template <typename Class>
	const MemberInfo& RegisterField(TypeId classId, VariableId MemberId, const std::string_view fieldName, uint32_t Offset, uint32_t Size, uint32_t Align)
	{
		MemberInfo info{};
		info.Name = fieldName;
		info.VariableId = MemberId;
		info.Offset = Offset;
		info.Size = Size;
		info.Align = Align;

		auto& memberInfo = const_cast<TypeInfo&>(RegisterType<Class>());

		return *memberInfo.Members.emplace(std::move(info)).first;
	}

	// TODO fix this
	template<typename Class, typename Field>
	const MemberInfo& RegisterField(const std::string_view fieldName, uint32_t Offset)
	{
		auto registerClass = RegisterType<Class>();
		auto registerField = RegisterType<Field>();

		return RegisterField(
			TypeId::Create<Class>(),
			VariableId::Create<Field>(),
			fieldName,
			Offset,
			sizeof(Field),
			alignof(Field));
	}

	// TODO fix this
	template<typename Field>
	const MemberInfo& RegisterField(TypeId classId, const std::string_view fieldName, uint32_t Offset)
	{
		auto registerField = RegisterType<Field>();

		return RegisterField(
			classId,
			VariableId::Create<Field>(),
			fieldName,
			Offset,
			sizeof(Field),
			alignof(Field));
	}

}