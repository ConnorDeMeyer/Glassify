#pragma once

#include <algorithm>

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
	constexpr uint64_t hash(std::string_view str)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (char c : str)
		{
			hash_value ^= static_cast<uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}

	constexpr uint64_t hash(std::span<const uint64_t> span)
	{
		std::uint64_t hash_value = 0xcbf29ce484222325ULL;
		constexpr std::uint64_t prime = 0x100000001b3ULL;
		for (auto c : span)
		{
			hash_value ^= static_cast<const uint64_t>(c);
			hash_value *= prime;
		}
		return hash_value;
	}

	constexpr uint32_t VariableId::GetSize() const
	{
		return IsRefOrPointer() ? static_cast<uint32_t>(sizeof(void*)) : GetTypeId().GetInfo().Size * GetArraySize();
	}

	constexpr uint32_t VariableId::GetAlign() const
	{
		return IsRefOrPointer() ? static_cast<uint32_t>(alignof(void*)) : GetTypeId().GetInfo().Align;
	}

	constexpr bool operator==(TypeId lhs, TypeId rhs)
	{
		return lhs.GetId() == rhs.GetId();
	}

	constexpr bool operator==(FunctionId lhs, FunctionId rhs)
	{
		return lhs.GetId() == rhs.GetId();
	}

	constexpr bool operator==(const VariableId& lhs, const VariableId& rhs)
	{
		return lhs.Type == rhs.Type &&
			lhs.ArraySize == rhs.ArraySize &&
			lhs.PointerAmount == rhs.PointerAmount &&
			lhs.TraitFlags == rhs.TraitFlags;
	}

	inline std::ostream& operator<<(std::ostream& lhs, TypeId rhs)
	{
		lhs << rhs.GetId();

		return lhs;
	}

	inline std::istream& operator>>(std::istream& lhs, TypeId rhs)
	{
		uint64_t idInt{};
		lhs >> idInt;
		rhs.SetTypeId(idInt);

		return lhs;
	}

	inline std::ostream& operator<<(std::ostream& lhs, const VariableId& rhs)
	{
		lhs << rhs.Type << ' '
			<< rhs.ArraySize << ' '
			<< rhs.PointerAmount << ' '
			<< rhs.TraitFlags;

		return lhs;
	}

	inline std::istream& operator>>(std::istream& lhs, const VariableId& rhs)
	{
		lhs >> rhs.Type
			>> rhs.ArraySize
			>> rhs.PointerAmount
			>> rhs.TraitFlags;

		return lhs;
	}

	template <typename T>
	constexpr TypeId TypeId::Create()
	{
		AutoRegisterTypeOnce<T>();
		return TypeId(TypeHash<strip_type_t<T>>());
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

	inline std::unordered_map<TypeId, TypeInfo>& GetTypeInfoMap()
	{
		static std::unordered_map<TypeId, TypeInfo> TypeInfoMap;
		return TypeInfoMap;
	}

	inline const std::unordered_map<TypeId, TypeInfo>& GetAllTypeInfo()
	{
		return GetTypeInfoMap();
	}

	template <typename ... Types>
	constexpr std::array<VariableId, sizeof...(Types)> GetVariableArray()
	{
		return std::array<VariableId, sizeof...(Types)> {VariableId::Create<Types>()...};
	}

	template <typename T>
	const TypeInfo& RegisterType()
	{
		auto& typeInfoMap = GetTypeInfoMap();
		
		constexpr TypeId hash = TypeId::Create<T>();

		const auto it = typeInfoMap.find(hash);
		if (it == typeInfoMap.end())
		{
			return typeInfoMap.emplace(
				hash,
				TypeInfo::Create<T>()
			).first->second;
		}
		return it->second;
	}

	inline const TypeInfo& TypeId::GetInfo() const
	{
		return GetTypeInfo(*this);
	}

	inline const TypeInfo& GetTypeInfo(TypeId id)
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
	const MemberInfo& RegisterField(VariableId MemberId, std::string_view fieldName, uint32_t Offset, uint32_t Size, uint32_t Align)
	{
		MemberInfo info;
		info.Name			= fieldName;
		info.VariableId		= MemberId;
		info.Offset			= Offset;
		info.Size			= Size;
		info.Align			= Align;

		auto& memberInfo = const_cast<TypeInfo&>(RegisterType<Class>());

		return *memberInfo.Members.emplace(info).first;
	}

	template<typename Class, typename Field>
	const MemberInfo& RegisterField(std::string_view fieldName, uint32_t Offset)
	{
		auto registerField = RegisterType<Field>();

		return RegisterField<Class>(
			TypeId::Create<Class>(),
			VariableId::Create<Field>(),
			fieldName,
			Offset,
			sizeof(Field),
			alignof(Field));
	}

	/** FUNCTION REFLECTION*/

	struct GlobalFunctionsData
	{
		std::unordered_map<FunctionId, FunctionInfo> FunctionInfoMap{};
		std::unordered_map<std::string_view, FunctionId> NameToIdMap{};
	};

	inline GlobalFunctionsData& GetGlobalFunctionsData()
	{
		static GlobalFunctionsData globalFunctionData{};
		return globalFunctionData;
	}

	template <typename ReturnT, typename ... ParameterTs>
	auto FunctionInfo::Cast() const -> ReturnT(*)(ParameterTs...)
	{
		constexpr uint64_t typesHash = GetTypesHash<ReturnT, ParameterTs...>();

		return (TypesHash == typesHash) ?
			reinterpret_cast<ReturnT(*)(ParameterTs...)>(FunctionAddress) :
			nullptr;
	}

	inline std::string VariableId::ToString() const
	{
		std::string name = std::string(Type.GetInfo().Name);

		if (IsVolatile()) name = "volatile " + name;
		if (IsConst()) name = "const " + name;

		const uint32_t pointerAmount = GetPointerAmount();
		for (uint32_t i{}; i < pointerAmount; ++i)
		{
			name += '*';
		}

		if (IsRValReference()) name += "&&";
		else if (IsReference()) name += '&';

		return name;
	}

	inline void FunctionInfo::Call(const Storage::TypeTuple& typeTuple, void* pReturnValue) const
	{
		assert(typeTuple.GetVariableIds().size() == ParameterTypes.size());
		assert(std::equal(ParameterTypes.begin(), ParameterTypes.end(), typeTuple.GetVariableIds().begin()));
		FunctionCaller(FunctionAddress, typeTuple, pReturnValue);
	}

	inline const FunctionInfo& FunctionId::GetInfo() const
	{
		return GetGlobalFunctionsData().FunctionInfoMap[*this];
	}

	inline void FunctionId::Call(const Storage::TypeTuple& typeTuple, void* pReturnValue) const
	{
		GetInfo().Call(typeTuple, pReturnValue);
	}

	template <typename ReturnType, typename ... ParameterTypes>
	auto FunctionId::Cast() const -> ReturnType(*)(ParameterTypes...)
	{
		return GetInfo().Cast<ReturnType, ParameterTypes...>();
	}

	template <typename ReturnType, typename ... ParameterTypes>
	FunctionId FunctionId::Create(ReturnType(*function)(ParameterTypes...), std::string_view name)
	{
		return FunctionId{ GetFunctionHash(function, name) };
	}

#ifdef GLAS_STORAGE
	template <typename ParameterTypesTuple, typename Function, size_t... Index>
	auto TupleFunctionCall(Function function, const Storage::TypeTuple& tupleTuple, std::index_sequence<Index...>)
	{
		return function(tupleTuple.Get<std::tuple_element_t<Index, ParameterTypesTuple>>(Index)...);
	}
#endif

	template <typename ReturnType, typename ... ParameterTypes>
	const FunctionInfo& RegisterFunction(ReturnType(*function)(ParameterTypes...), std::string_view name)
	{
		auto& globalFunctionData = GetGlobalFunctionsData();

		FunctionId functionId = FunctionId::Create(function, name);

		const auto it = globalFunctionData.FunctionInfoMap.find(functionId);
		if (it != globalFunctionData.FunctionInfoMap.end())
			return it->second;

		FunctionInfo info{};
		info.FunctionAddress	= reinterpret_cast<const void*>(function);
		info.ReturnType			= VariableId::Create<ReturnType>();
		info.Name				= name;
		info.TypesHash			= GetTypesHash<ReturnType, ParameterTypes...>();

		if constexpr (sizeof...(ParameterTypes) != 0)
		{
			info.ParameterTypes.resize(sizeof...(ParameterTypes));
			auto parameterTypes = GetVariableArray<ParameterTypes...>();

			std::copy(parameterTypes.begin(), parameterTypes.end(), info.ParameterTypes.begin());
		}

#ifdef GLAS_STORAGE
		info.FunctionCaller = [](const void* address, const Storage::TypeTuple& tupleStorage, void* returnAddress) -> void
		{
			if constexpr (std::is_same_v<ReturnType, void>)
			{
				TupleFunctionCall<std::tuple<ParameterTypes...>>(
					reinterpret_cast<decltype(function)>(address),
					tupleStorage,
					std::make_index_sequence<sizeof...(ParameterTypes)>());
			}
			else
			{
				if (returnAddress)
					*static_cast<ReturnType*>(returnAddress) = TupleFunctionCall<std::tuple<ParameterTypes...>>(
						reinterpret_cast<decltype(function)>(address),
						tupleStorage,
						std::make_index_sequence<sizeof...(ParameterTypes)>());
				else
					TupleFunctionCall<std::tuple<ParameterTypes...>>(
						reinterpret_cast<decltype(function)>(address),
						tupleStorage,
						std::make_index_sequence<sizeof...(ParameterTypes)>());
			}
		};
#endif

		globalFunctionData.NameToIdMap.emplace(name, functionId);
		return globalFunctionData.FunctionInfoMap.emplace(functionId, std::move(info)).first->second;
	}

	template <typename... Types>
	constexpr uint64_t GetTypesHash()
	{
		constexpr auto variableIds = GetVariableArray<Types...>();

		std::array<uint64_t, sizeof...(Types)> variableHashes{};

		std::transform(variableIds.begin(), variableIds.end(), variableHashes.begin(), [](VariableId id)
			{
				return id.GetHash();
			});

		return hash({ variableHashes.data(), variableHashes.size() });
	}

	template <typename ReturnType, typename ... ParameterTypes>
	uint64_t GetFunctionHash(ReturnType(*)(ParameterTypes...), std::string_view name)
	{
		return hash(name) ^ GetTypesHash<ReturnType, ParameterTypes...>();
	}
}
