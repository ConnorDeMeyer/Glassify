#pragma once

#ifdef GLAS_SERIALIZATION_JSON

#include "../glas_decl.h"
#include "glas_serialization_config_json.h"

#include <functional>
#include <type_traits>

#include "Json3rdParty/rapidjson/rapidjson.h"
#include "Json3rdParty/rapidjson/document.h"
#include "Json3rdParty/rapidjson/istreamwrapper.h"
#include "Json3rdParty/rapidjson/ostreamwrapper.h"
#include "Json3rdParty/rapidjson/prettywriter.h"

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoJSon(TypeInfo& info)
	{
		if constexpr (CustomJSonSerializer<T>)
		{
			info.JSonSerializer = [](rapidjson::Value& jsonVal, const void* data, RapidJsonAllocator& allocator)
				{
					JSonSerializer<T>::Serialize(jsonVal, *static_cast<const T*>(data), allocator);
				};

			info.JSonDeserializer = [](rapidjson::Value& jsonVal, void* data)
				{
					JSonSerializer<T>::Deserialize(jsonVal, *static_cast<T*>(data));
				};
		}
		else
		{
			info.JSonSerializer = [](rapidjson::Value& jsonVal, const void* data, RapidJsonAllocator& allocator)
				{
					SerializeJSonDefault(jsonVal, *static_cast<const T*>(data), allocator);
				};

			info.JSonDeserializer = [](rapidjson::Value& jsonVal, void* data)
				{
					DeserializeJSonDefault(jsonVal, *static_cast<T*>(data));
				};
		}
	}

	inline void SerializeJSon(std::ostream& stream, const void* data, TypeId type)
	{
		rapidjson::Document document;

		document.SetObject();

		auto& info = type.GetInfo();
		assert(info.JSonSerializer);
		info.JSonSerializer(document, data, document.GetAllocator());

		if (document.HasParseError())
		{
			std::cout << "Error " << document.GetParseError();
		}

		rapidjson::OStreamWrapper streamWrapper(stream);
		rapidjson::PrettyWriter prettyWriter{ streamWrapper };
		document.Accept(prettyWriter);
	}

	inline void DeserializeJSon(std::istream& stream, void* data, TypeId type)
	{
		rapidjson::Document document;

		rapidjson::IStreamWrapper streamWrapper(stream);

		document.ParseStream(streamWrapper);

		auto& info = type.GetInfo();
		assert(info.JSonDeserializer);
		info.JSonDeserializer(document, data);

		if (document.HasParseError())
		{
			std::cout << "Error " << document.GetParseError();
		}
	}

	template <typename T>
	void SerializeJSon(std::ostream& stream, const T& value)
	{
		SerializeJSon(stream, &value, TypeId::Create<T>());
	}

	template <typename T>
	void DeserializeJSon(std::istream& stream, T& value)
	{
		DeserializeJSon(stream, &value, TypeId::Create<T>());
	}

	template <typename T>
	void SerializeJSon(rapidjson::Value& jsonVal, const T& value, RapidJsonAllocator& allocator)
	{
		if constexpr (CustomJSonSerializer<T>)
			JSonSerializer<T>::Serialize(jsonVal, value, allocator);
		else
			SerializeJSonDefault(jsonVal, value, allocator);
	}

	template <typename T>
	void DeserializeJSon(rapidjson::Value& jsonVal, T& value)
	{
		if constexpr (CustomJSonSerializer<T>)
			JSonSerializer<T>::Deserialize(jsonVal, value);
		else
			DeserializeJSonDefault(jsonVal, value);
	}

	inline void SerializeJSonDefault(rapidjson::Value& jsonVal, const void* data, glas::TypeId type, RapidJsonAllocator& allocator)
	{
		auto& info = type.GetInfo();
		auto& members = info.Members;

		for (auto& member : members)
		{
			if (!member.Variable.IsRefOrPointer())
			{
				rapidjson::Value MemberVal{ rapidjson::kObjectType };

				auto& memberInfo = member.Variable.GetTypeId().GetInfo();
				memberInfo.JSonSerializer(MemberVal, VoidOffset(data, member.Offset), allocator);

				jsonVal.AddMember(rapidjson::GenericStringRef{ member.Name.c_str() }, MemberVal, allocator);
			}
		}
	}

	inline void DeserializeJSonDefault(rapidjson::Value& jsonVal, void* data, glas::TypeId type)
	{
		auto& info = type.GetInfo();
		auto& members = info.Members;

		for (auto& member : members)
		{
			if (!member.Variable.IsRefOrPointer())
			{
				auto memberVal = jsonVal.FindMember(member.Name);

				if (memberVal != jsonVal.MemberEnd())
				{
					auto& memberInfo = member.Variable.GetTypeId().GetInfo();
					memberInfo.JSonDeserializer(memberVal->value, VoidOffset(data, member.Offset));
				}
			}
		}
	}

	template <typename T>
	void SerializeJSonDefault(rapidjson::Value& jsonVal, const T& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonDefault(jsonVal, static_cast<const void*>(&value), TypeId::Create<T>(), allocator);
	}

	template <typename T>
	void DeserializeJSonDefault(rapidjson::Value& jsonVal, T& value)
	{
		DeserializeJSonDefault(jsonVal, static_cast<void*>(&value), TypeId::Create<T>());
	}

	/** FLOAT */
	inline void JSonSerializer<float>::Serialize(rapidjson::Value& jsonVal, const float& value, RapidJsonAllocator&)
	{
		jsonVal.SetFloat(value);
	}

	inline void JSonSerializer<float>::Deserialize(rapidjson::Value& jsonVal, float& value)
	{
		value = jsonVal.GetFloat();
	}

	/** DOUBLE */
	inline void JSonSerializer<double>::Serialize(rapidjson::Value& jsonVal, const double& value, RapidJsonAllocator&)
	{
		jsonVal.SetDouble(value);
	}

	inline void JSonSerializer<double>::Deserialize(rapidjson::Value& jsonVal, double& value)
	{
		value = jsonVal.GetDouble();
	}

	/** INT */
	inline void JSonSerializer<int>::Serialize(rapidjson::Value& jsonVal, const int& value, RapidJsonAllocator&)
	{
		jsonVal.SetInt(value);
	}

	inline void JSonSerializer<int>::Deserialize(rapidjson::Value& jsonVal, int& value)
	{
		value = jsonVal.GetInt();
	}

	/** INT 64 */
	inline void JSonSerializer<int64_t>::Serialize(rapidjson::Value& jsonVal, const int64_t& value, RapidJsonAllocator& )
	{
		jsonVal.SetInt64(value);
	}

	inline void JSonSerializer<int64_t>::Deserialize(rapidjson::Value& jsonVal, int64_t& value)
	{
		value = jsonVal.GetInt64();
	}

	/** UINT */
	inline void JSonSerializer<unsigned int>::Serialize(rapidjson::Value& jsonVal, const unsigned int& value, RapidJsonAllocator& )
	{
		jsonVal.SetUint(value);
	}

	inline void JSonSerializer<unsigned int>::Deserialize(rapidjson::Value& jsonVal, unsigned& value)
	{
		value = jsonVal.GetUint();
	}

	/** UINT 64 */
	inline void JSonSerializer<uint64_t>::Serialize(rapidjson::Value& jsonVal, const uint64_t& value, RapidJsonAllocator& )
	{
		jsonVal.SetUint64(value);
	}

	inline void JSonSerializer<uint64_t>::Deserialize(rapidjson::Value& jsonVal, uint64_t& value)
	{
		value = jsonVal.GetInt64();
	}

	/** BOOL */
	inline void JSonSerializer<bool>::Serialize(rapidjson::Value& jsonVal, const bool& value, RapidJsonAllocator& )
	{
		jsonVal.SetBool(value);
	}

	inline void JSonSerializer<bool>::Deserialize(rapidjson::Value& jsonVal, bool& value)
	{
		value = jsonVal.GetBool();
	}

	/** STRING */
	inline void JSonSerializer<char*>::Serialize(rapidjson::Value& jsonVal, const char*& value, RapidJsonAllocator& allocator)
	{
		jsonVal.SetString(value, allocator);
	}

	inline void JSonSerializer<char*>::Deserialize(rapidjson::Value& jsonVal, const char*& value)
	{
		value = jsonVal.GetString();
	}

	inline void JSonSerializer<TypeId>::Serialize(rapidjson::Value& jsonVal, const TypeId& value, RapidJsonAllocator&)
	{
		jsonVal.SetUint64(value.GetId());
	}

	inline void JSonSerializer<TypeId>::Deserialize(rapidjson::Value& jsonVal, TypeId& value)
	{
		value.SetTypeId(jsonVal.GetUint64());
	}

	inline void JSonSerializer<VariableId>::Serialize(rapidjson::Value& jsonVal, const VariableId& value, RapidJsonAllocator& allocator)
	{
		rapidjson::Value typeVal{};
		rapidjson::Value constFlag{};
		rapidjson::Value volatileFlag{};
		rapidjson::Value referenceFlag{};
		rapidjson::Value rValReferenceFlag{};
		rapidjson::Value pointerAmountFlag{};
		rapidjson::Value arraySizeFlag{};

		typeVal.SetUint64(value.GetTypeId().GetId());
		constFlag.SetBool(value.IsConst());
		volatileFlag.SetBool(value.IsVolatile());
		referenceFlag.SetBool(value.IsReference());
		rValReferenceFlag.SetBool(value.IsRValReference());
		pointerAmountFlag.SetUint(value.GetPointerAmount());
		arraySizeFlag.SetUint(value.GetArraySize());

		jsonVal.AddMember("Type", typeVal, allocator);
		jsonVal.AddMember("Const", constFlag, allocator);
		jsonVal.AddMember("Volatile", volatileFlag, allocator);
		jsonVal.AddMember("Reference", referenceFlag, allocator);
		jsonVal.AddMember("R Value", rValReferenceFlag, allocator);
		jsonVal.AddMember("Pointer Amount", pointerAmountFlag, allocator);
		jsonVal.AddMember("Array Size", arraySizeFlag, allocator);
	}

	inline void JSonSerializer<VariableId>::Deserialize(rapidjson::Value& jsonVal, VariableId& value)
	{
		auto typeVal = jsonVal.FindMember("Type");
		auto constVal = jsonVal.FindMember("Const");
		auto volatileVal = jsonVal.FindMember("Volatile");
		auto referenceVal = jsonVal.FindMember("Reference");
		auto rValRefVal = jsonVal.FindMember("R Value");
		auto pointerVal = jsonVal.FindMember("Pointer Amount");
		auto arrayVal = jsonVal.FindMember("Array Size");

		if (typeVal != jsonVal.MemberEnd())
		{
			TypeId id{};
			JSonSerializer<TypeId>::Deserialize(typeVal->value, id);
			value.SetTypeId(id);
		}
		if (constVal != jsonVal.MemberEnd())
		{
			if (constVal->value.GetBool()) value.SetConstFlag();
			else value.RemoveConstFlag();
		}
		if (volatileVal != jsonVal.MemberEnd())
		{
			if (volatileVal->value.GetBool()) value.SetVolatileFlag();
			else value.RemoveVolatileFlag();
		}
		if (referenceVal != jsonVal.MemberEnd())
		{
			if (referenceVal->value.GetBool()) value.SetReferenceFlag();
			else value.RemoveReferenceFlag();
		}
		if (rValRefVal != jsonVal.MemberEnd())
		{
			if (rValRefVal->value.GetBool()) value.SetRValReferenceFlag();
			else value.RemoveRValReferenceFlag();
		}
		if (pointerVal != jsonVal.MemberEnd())
		{
			value.SetPointerAmount(static_cast<uint16_t>(pointerVal->value.GetUint()));
		}
		if (arrayVal != jsonVal.MemberEnd())
		{
			value.SetArraySize(arrayVal->value.GetUint());
		}
	}

	inline void JSonSerializer<FunctionId>::Serialize(rapidjson::Value& jsonVal, const FunctionId& value, RapidJsonAllocator&)
	{
		jsonVal.SetUint64(value.GetId());
	}

	inline void JSonSerializer<FunctionId>::Deserialize(rapidjson::Value& jsonVal, FunctionId& value)
	{
		value.SetId(jsonVal.GetUint64());
	}

	template <typename T, typename Container>
	void SerializeJSonContainer(rapidjson::Value& jsonVal, const Container& container, RapidJsonAllocator& allocator)
	{
		jsonVal.SetArray();

		for (auto& element : container)
		{
			auto arrayElement = rapidjson::Value(rapidjson::kObjectType);

			SerializeJSon(arrayElement, element, allocator);

			jsonVal.PushBack(arrayElement, allocator);
		}
	}

	template <typename T, typename Adder>
	void DeserializeJSonContainer(rapidjson::Value& jsonVal, Adder adder)
	{
		for (auto& element : jsonVal.GetArray())
		{
			T value{};
			DeserializeJSon(element, value);

			adder(value);
		}
	}

	template <typename T>
	void JSonSerializer<T[]>::Serialize(rapidjson::Value& jsonVal, const T value[], RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T>
	void JSonSerializer<T[]>::Deserialize(rapidjson::Value& jsonVal, T value[])
	{
		size_t counter{};
		for (auto& element : jsonVal.GetArray())
		{
			DeserializeJSon(element, value[counter]);
			++counter;
		}
	}
#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	void JSonSerializer<std::basic_string<Elem, Traits, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::basic_string<Elem, Traits, Alloc>& value, RapidJsonAllocator& allocator)
	{
		jsonVal.SetString(rapidjson::GenericStringRef(value.c_str()), allocator);
	}

	template <typename Elem, typename Traits, typename Alloc>
	void JSonSerializer<std::basic_string<Elem, Traits, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::basic_string<Elem, Traits, Alloc>& value)
	{
		value = jsonVal.GetString();
	}
#endif
#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	void JSonSerializer<std::vector<T, Alloc>>::Serialize(rapidjson::Value& jsonVal, const std::vector<T, Alloc>& value,
		RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, typename Alloc>
	void JSonSerializer<std::vector<T, Alloc>>::Deserialize(rapidjson::Value& jsonVal, std::vector<T, Alloc>& value)
	{
		DeserializeJSonContainer<T>(jsonVal, [&value](T& element) { value.push_back(element); });
	}
#endif
#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	void JSonSerializer<std::array<T, Size>>::Serialize(rapidjson::Value& jsonVal, const std::array<T, Size>& value,
		RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, size_t Size>
	void JSonSerializer<std::array<T, Size>>::Deserialize(rapidjson::Value& jsonVal, std::array<T, Size>& value)
	{
		size_t counter{};
		for (auto& element : jsonVal.GetArray())
		{
			DeserializeJSon(element, value[counter]);
			++counter;
		}
	}
#endif
#ifdef _DEQUE_
	/** DEQUE*/
	template <typename T, typename Alloc>
	void JSonSerializer<std::deque<T, Alloc>>::Serialize(rapidjson::Value& jsonVal, const std::deque<T, Alloc>& value,
		RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, typename Alloc>
	void JSonSerializer<std::deque<T, Alloc>>::Deserialize(rapidjson::Value& jsonVal, std::deque<T, Alloc>& value)
	{
		DeserializeJSonContainer<T>(jsonVal, [&value](T& element) { value.push_back(element); });
	}
#endif
#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	void JSonSerializer<std::forward_list<T, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::forward_list<T, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, typename Alloc>
	void JSonSerializer<std::forward_list<T, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::forward_list<T, Alloc>& value)
	{
		DeserializeJSonContainer<T>(jsonVal, [&value](T& element) { value.push_front(element); });
	}
#endif
#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	void JSonSerializer<std::list<T, Alloc>>::Serialize(rapidjson::Value& jsonVal, const std::list<T, Alloc>& value,
		RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, typename Alloc>
	void JSonSerializer<std::list<T, Alloc>>::Deserialize(rapidjson::Value& jsonVal, std::list<T, Alloc>& value)
	{
		DeserializeJSonContainer<T>(jsonVal, [&value](T& element) { value.push_back(element); });
	}
#endif
#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::unordered_set<T, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::unordered_set<T, Hasher, Keyeq, Alloc>& value)
	{
		DeserializeJSonContainer<T>(jsonVal, [&value](T& element) { value.insert(element); });
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<T>(jsonVal, value, allocator);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value)
	{
		DeserializeJSonContainer<T>(jsonVal, [&value](T& element) { value.insert(element); });
	}
#endif
#ifdef _MAP_
	/** MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	void JSonSerializer<std::map<Key, Value, P, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::map<Key, Value, P, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<std::pair<Key, Value>>(jsonVal, value, allocator);
	}

	template <typename Key, typename Value, typename P, typename Alloc>
	void JSonSerializer<std::map<Key, Value, P, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::map<Key, Value, P, Alloc>& value)
	{
		DeserializeJSonContainer<std::pair<Key, Value>>(jsonVal, [&value](std::pair<Key, Value>& element) { value.insert(element); });
	}

	template <typename Key, typename Value, typename P, typename Alloc>
	void JSonSerializer<std::multimap<Key, Value, P, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::multimap<Key, Value, P, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<std::pair<Key, Value>>(jsonVal, value, allocator);
	}

	template <typename Key, typename Value, typename P, typename Alloc>
	void JSonSerializer<std::multimap<Key, Value, P, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::multimap<Key, Value, P, Alloc>& value)
	{
		DeserializeJSonContainer<std::pair<Key, Value>>(jsonVal, [&value](std::pair<Key, Value>& element) { value.insert(element); });
	}
#endif
#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<std::pair<Key, Value>>(jsonVal, value, allocator);
	}

	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>::Deserialize(rapidjson::Value& jsonVal,
		std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value)
	{
		DeserializeJSonContainer<std::pair<Key, Value>>(jsonVal, [&value](std::pair<Key, Value>& element) { value.insert(element); });
	}

	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>::Serialize(rapidjson::Value& jsonVal,
		const std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator)
	{
		SerializeJSonContainer<std::pair<Key, Value>>(jsonVal, value, allocator);
	}

	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void JSonSerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>::Deserialize(
		rapidjson::Value& jsonVal, std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value)
	{
		DeserializeJSonContainer<std::pair<Key, Value>>(jsonVal, [&value](std::pair<Key, Value>& element) { value.insert(element); });
	}
#endif
#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	void JSonSerializer<std::unique_ptr<T, Delete>>::Serialize(rapidjson::Value& jsonVal,
		const std::unique_ptr<T, Delete>& value, RapidJsonAllocator& allocator)
	{
		if (value)
		{
			rapidjson::Value ownedVal{ rapidjson::kObjectType };

			SerializeJSon(jsonVal, *value, allocator);

			ownedVal.AddMember("Value", ownedVal, allocator);
		}
		else
		{
			jsonVal.SetNull();
		}
	}

	template <typename T, typename Delete>
	void JSonSerializer<std::unique_ptr<T, Delete>>::Deserialize(rapidjson::Value& jsonVal,
		std::unique_ptr<T, Delete>& value)
	{
		if (!jsonVal.IsNull())
		{
			value = std::make_unique<T>();
			DeserializeJSon(jsonVal["Value"], *value);
		}
	}
#endif
#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	void JSonSerializer<std::optional<T>>::Serialize(rapidjson::Value& jsonVal, const std::optional<T>& value,
		RapidJsonAllocator& allocator)
	{
		if (value)
		{
			rapidjson::Value ownedVal{ rapidjson::kObjectType };

			SerializeJSon(jsonVal, value.value(), allocator);

			ownedVal.AddMember("Value", ownedVal, allocator);
		}
		else
		{
			jsonVal.SetNull();
		}
	}

	template <typename T>
	void JSonSerializer<std::optional<T>>::Deserialize(rapidjson::Value& jsonVal, std::optional<T>& value)
	{
		if (!jsonVal.IsNull())
		{
			auto& val = value.emplace();
			DeserializeJSon(jsonVal["Value"], val);
		}
	}
#endif
#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	void JSonSerializer<std::pair<T1, T2>>::Serialize(rapidjson::Value& jsonVal, const std::pair<T1, T2>& value,
		RapidJsonAllocator& allocator)
	{
		rapidjson::Value Val1{ rapidjson::kObjectType };
		rapidjson::Value Val2{ rapidjson::kObjectType };

		SerializeJSon(Val1, value.first, allocator);
		SerializeJSon(Val2, value.second, allocator);

		jsonVal.AddMember("First", Val1, allocator);
		jsonVal.AddMember("Second", Val2, allocator);
	}

	template <typename T1, typename T2>
	void JSonSerializer<std::pair<T1, T2>>::Deserialize(rapidjson::Value& jsonVal, std::pair<T1, T2>& value)
	{
		DeserializeJSon(jsonVal["First"], value.first);
		DeserializeJSon(jsonVal["Second"], value.second);
	}

	template <size_t Index, typename Tuple>
	void SerializeJSonTuple(rapidjson::Value& jsonVal, const Tuple& tuple, RapidJsonAllocator& allocator)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		rapidjson::Value name{ std::to_string(Index), allocator };
		rapidjson::Value val{ rapidjson::kObjectType };

		SerializeJSon(val, std::get<Index>(tuple), allocator);

		jsonVal.AddMember(name, val, allocator);

		if constexpr (Index + 1 < size)
		{
			SerializeJSonTuple<Index + 1, Tuple>(jsonVal, tuple, allocator);
		}
	}

	template <size_t Index, typename Tuple>
	void DeserializeJSonTuple(rapidjson::Value& jsonVal, Tuple& tuple)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		DeserializeJSon(jsonVal[std::to_string(Index)], std::get<Index>(tuple));

		if constexpr (Index + 1 < size)
		{
			DeserializeJSonTuple<Index + 1, Tuple>(jsonVal, tuple);
		}
	}

	template <typename ... Ts>
	void JSonSerializer<std::tuple<Ts...>>::Serialize(rapidjson::Value& jsonVal, const std::tuple<Ts...>& value,
		RapidJsonAllocator& allocator)
	{
		SerializeJSonTuple<0>(jsonVal, value, allocator);
	}

	template <typename ... Ts>
	void JSonSerializer<std::tuple<Ts...>>::Deserialize(rapidjson::Value& jsonVal, std::tuple<Ts...>& value)
	{
		DeserializeJSonTuple<0>(jsonVal, value);
	}
#endif
#ifdef GLAS_STORAGE
	/** TYPE STORAGE */
	inline void JSonSerializer<Storage::TypeStorage>::Serialize(rapidjson::Value& jsonVal,
		const Storage::TypeStorage& value, RapidJsonAllocator& allocator)
	{
		if (value.GetType().IsValid() && value.GetData())
		{
			rapidjson::Value typeVal{ rapidjson::kObjectType };

			SerializeJSon(typeVal, value.GetType(), allocator);

			jsonVal.AddMember("Type", typeVal, allocator);
		}
		else
		{
			rapidjson::Value typeVal{ rapidjson::kNullType };
			jsonVal.AddMember("Type", typeVal, allocator);
		}

		if (value.GetType().IsValid() && value.GetData())
		{
			rapidjson::Value dataVal{ rapidjson::kObjectType };

			SerializeJSonDefault(dataVal, value.GetData(), value.GetType(), allocator);

			jsonVal.AddMember("Data", dataVal, allocator);
		}
		else
		{
			rapidjson::Value dataVal{ rapidjson::kNullType };
			jsonVal.AddMember("Data", dataVal, allocator);
		}
	}

	inline void JSonSerializer<Storage::TypeStorage>::Deserialize(rapidjson::Value& jsonVal,
		Storage::TypeStorage& value)
	{
		auto& typeVal = jsonVal["Type"];
		auto& dataVal = jsonVal["Data"];

		if (!typeVal.IsNull())
		{
			TypeId id{};
			DeserializeJSon(typeVal, id);
			value = Storage::TypeStorage(id);

			if (!dataVal.IsNull())
			{
				DeserializeJSonDefault(dataVal, value.GetData(), id);
			}
		}
	}

	inline void JSonSerializer<Storage::TypeTuple>::Serialize(rapidjson::Value& jsonVal,
	                                                          const Storage::TypeTuple& value, RapidJsonAllocator& allocator)
	{
		jsonVal.SetArray();

		for (uint32_t i{}; i < value.GetSize(); ++i)
		{
			rapidjson::Value arrayVal{ rapidjson::kObjectType };

			rapidjson::Value variableVal{ rapidjson::kObjectType };
			rapidjson::Value dataVal{ rapidjson::kObjectType };

			SerializeJSon(variableVal, value.GetVariable(i), allocator);

			if (!value.GetVariable(i).IsRefOrPointer())
			{
				SerializeJSonDefault(dataVal, value.GetVoid(i), value.GetVariable(i).GetTypeId(), allocator);
			}
			else
			{
				dataVal.SetNull();
			}

			arrayVal.AddMember("Variable ID", variableVal, allocator);
			arrayVal.AddMember("Data", dataVal, allocator);

			jsonVal.PushBack(arrayVal, allocator);
		}
	}

	inline void JSonSerializer<Storage::TypeTuple>::Deserialize(rapidjson::Value& jsonVal, Storage::TypeTuple& value)
	{
		const auto tupleSize = jsonVal.Size();

		std::vector<VariableId> variables;
		variables.resize(tupleSize);

		auto jsonArray = jsonVal.GetArray();

		size_t counter{};
		for (auto& arrayElement : jsonArray)
		{
			DeserializeJSon(arrayElement["Variable ID"], variables[counter]);
			++counter;
		}

		value = Storage::TypeTuple(std::span(variables));

		counter = 0;
		for (auto& arrayElement : jsonArray)
		{
			if (!arrayElement.IsNull())
			{
				DeserializeJSonDefault(arrayElement["Data"], value.GetVoid(counter), variables[counter].GetTypeId());
			}
			++counter;
		}
	}

	inline void JSonSerializer<Storage::TypeVector>::Serialize(rapidjson::Value& jsonVal,
	                                                           const Storage::TypeVector& value, RapidJsonAllocator& allocator)
	{
		rapidjson::Value typeVal{ rapidjson::kObjectType };
		rapidjson::Value arrayVal{ rapidjson::kArrayType };

		SerializeJSon(typeVal, value.GetType(), allocator);

		if (value.GetType().IsValid())
		{
			const auto jsonSerializer = value.GetType().GetInfo().JSonSerializer;

			for (const auto element : value)
			{
				rapidjson::Value arrayElement{ rapidjson::kObjectType };

				jsonSerializer(arrayElement, element, allocator);

				arrayVal.PushBack(arrayElement, allocator);
			}
		}

		jsonVal.AddMember("Type ID", typeVal, allocator);
		jsonVal.AddMember("Data", arrayVal, allocator);
	}

	inline void JSonSerializer<Storage::TypeVector>::Deserialize(rapidjson::Value& jsonVal, Storage::TypeVector& value)
	{
		auto vectorType = TypeId{ jsonVal["Type ID"].GetUint64() };
		auto jsonArray = jsonVal["Data"].GetArray();

		value = Storage::TypeVector(vectorType);

		for (auto& jsonElement : jsonArray)
		{
			auto element = value.PushBack();
			DeserializeJSonDefault(jsonElement, element, value.GetType());
		}
	}
#endif

}

#endif