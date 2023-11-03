#pragma once

#ifdef GLAS_SERIALIZATION_BINARY

#include "../glas_decl.h"
#include "glas_serialization_config_binary.h"

#include <sstream>
#include <iostream>
#include <functional>
#include <type_traits>
#include <cassert>

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoBinary(TypeInfo& info)
	{
		info.BinarySerializer = [](std::ostream& stream, const void* data) { BinarySerializer<T>::Serialize(stream, *static_cast<const T*>(data)); };
		info.BinaryDeserializer = [](std::istream& stream, void* data) { BinarySerializer<T>::Deserialize(stream, *static_cast<T*>(data)); };
	}

	/** HELPER FUNCTIONS */

	template <typename T>
	void WriteStream(std::ostream& stream, const T& value)
	{
		stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
	}

	template <typename T>
	T ReadStream(std::istream& stream)
	{
		T t;
		stream.read(reinterpret_cast<char*>(&t), sizeof(T));
		return t;
	}

	template <typename T>
	void SerializeBinary(std::ostream& stream, const T& value)
	{
		BinarySerializer<T>::Serialize(stream, value);
	}

	template <typename T>
	void DeserializeBinary(std::istream& stream, T& value)
	{
		BinarySerializer<T>::Deserialize(stream, value);
	}

	/** DEFAULT SERIALIZATION WITH MEMBERS */
	inline void SerializeBinaryDefault(std::ostream& stream, const void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!!(member.Properties & MemberProperties::Serializable) && !member.Variable.IsRefOrPointer())
			{
				member.Variable.GetTypeId().GetInfo().BinarySerializer(stream, VoidOffset(data, member.Offset));
			}
		}
	}

	inline void DeserializeBinaryDefault(std::istream& stream, void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!!(member.Properties & MemberProperties::Serializable) && !member.Variable.IsRefOrPointer())
			{
				member.Variable.GetTypeId().GetInfo().BinaryDeserializer(stream, VoidOffset(data, member.Offset));
			}
		}
	}

	/** DEFAULT SERIALIZATION */
	template <typename T>
	void BinarySerializer<T>::Serialize(std::ostream& stream, const T& value)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			WriteStream(stream, value);
		}
		else
		{
			SerializeBinaryDefault(stream, &value, TypeId::Create<T>());
		}
	}
	
	template <typename T>
	void BinarySerializer<T>::Deserialize(std::istream& stream, T& value)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			value = ReadStream<T>(stream);
		}
		else
		{
			DeserializeBinaryDefault(stream, &value, TypeId::Create<T>());
		}
	}

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	void BinarySerializer<std::basic_string<Elem, Traits, Alloc>>::Serialize(std::ostream& stream, const std::basic_string<Elem, Traits, Alloc>& value)
	{
		WriteStream(stream, value.size());
		stream.write(value.data(), value.size() * sizeof(Elem));
	}

	template <typename Elem, typename Traits, typename Alloc>
	void BinarySerializer<std::basic_string<Elem, Traits, Alloc>>::Deserialize(std::istream& stream, std::basic_string<Elem, Traits, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();
		value.resize(size);

		stream.read(value.data(), size * sizeof(Elem));
	}
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	void BinarySerializer<std::vector<T, Alloc>>::Serialize(std::ostream& stream, const std::vector<T, Alloc>& value)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			WriteStream(stream, value.size());
			stream.write(reinterpret_cast<const char*>(value.data()), value.size() * sizeof(T));
		}
		else
		{
			WriteStream(stream, value.size());

			for (auto& element : value)
			{
				BinarySerializer<T>::Serialize(stream, element);
			}
		}
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::vector<T, Alloc>>::Deserialize(std::istream& stream, std::vector<T, Alloc>& value)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			size_t size = ReadStream<size_t>(stream);

			value.clear();
			value.resize(size);

			stream.read(reinterpret_cast<char*>(value.data()), size * sizeof(T));
		}
		else
		{
			size_t size = ReadStream<size_t>(stream);

			value.clear();
			value.reserve(size);

			for (size_t i{}; i < size; ++i)
			{
				auto& element = value.emplace_back();
				BinarySerializer<T>::Deserialize(stream, element);
			}
		}
	}

	/** BOOLEAN VECTOR */
	template <typename Alloc>
	void BinarySerializer<std::vector<bool, Alloc>>::Serialize(std::ostream& stream, const std::vector<bool, Alloc>& value)
	{
		WriteStream(stream, value.size());
		stream.write(reinterpret_cast<const char*>(value.data()), value.size() / 8);
	}

	template <typename Alloc>
	void BinarySerializer<std::vector<bool, Alloc>>::Deserialize(std::istream& stream, std::vector<bool, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();
		value.resize(size);

		stream.read(reinterpret_cast<char*>(value.data()), size / 8);
	}

#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	void BinarySerializer<std::array<T, Size>>::Serialize(std::ostream& stream, const std::array<T, Size>& value)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			WriteStream(stream, value);
		}
		else
		{
			for (auto& element : value)
				BinarySerializer<T>::Serialize(stream, element);
		}
	}

	template <typename T, size_t Size>
	void BinarySerializer<std::array<T, Size>>::Deserialize(std::istream& stream, std::array<T, Size>& value)
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			value = ReadStream<std::array<T, Size>>(stream);
		}
		else
		{
			for (auto& element : value)
				BinarySerializer<T>::Deserialize(stream, element);
		}
	}

#endif

#ifdef _DEQUE_
	/** DEQUE */
	template <typename T, typename Alloc>
	void BinarySerializer<std::deque<T, Alloc>>::Serialize(std::ostream& stream, const std::deque<T, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<T>::Serialize(stream, element);
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::deque<T, Alloc>>::Deserialize(std::istream& stream, std::deque<T, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto& element = value.emplace_back();
			BinarySerializer<T>::Deserialize(stream, element);
		}
	}
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	void BinarySerializer<std::forward_list<T, Alloc>>::Serialize(std::ostream& stream, const std::forward_list<T, Alloc>& value)
	{
		size_t size = std::distance(value.begin(), value.end());
		WriteStream(stream, size);

		for (auto& element : value)
			BinarySerializer<T>::Serialize(stream, element);
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::forward_list<T, Alloc>>::Deserialize(std::istream& stream, std::forward_list<T, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto& element = value.emplace_front();
			BinarySerializer<T>::Deserialize(stream, element);
		}
	}
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	void BinarySerializer<std::list<T, Alloc>>::Serialize(std::ostream& stream, const std::list<T, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<T>::Serialize(stream, element);
	}

	template <typename T, typename Alloc>
	void BinarySerializer<std::list<T, Alloc>>::Deserialize(std::istream& stream, std::list<T, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto& element = value.emplace_back();
			BinarySerializer<T>::Deserialize(stream, element);
		}
	}
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Serialize(std::ostream& stream, const std::unordered_set<T, Hasher, Keyeq, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<T>::Serialize(stream, element);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>::Deserialize(std::istream& stream, std::unordered_set<T, Hasher, Keyeq, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto element = T{};
			BinarySerializer<T>::Deserialize(stream, element);
			value.insert(element);
		}
	}

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Serialize(std::ostream& stream, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<T>::Serialize(stream, element);
	}

	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>::Deserialize(std::istream& stream, std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto element = T{};
			BinarySerializer<T>::Deserialize(stream, element);
			value.insert(element);
		}
	}
#endif

#ifdef _MAP_
	/** MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::map<Key, Value, P, Alloc>>::Serialize(std::ostream& stream, const std::map<Key, Value, P, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<std::pair<Key, Value>>::Serialize(stream, element);
	}

	template <typename Key, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::map<Key, Value, P, Alloc>>::Deserialize(std::istream& stream, std::map<Key, Value, P, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto element = std::pair<Key, Value>{};
			BinarySerializer<std::pair<Key, Value>>::Deserialize(stream, element);
			value.insert(element);
		}
	}

	/** MULTI MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::multimap<Key, Value, P, Alloc>>::Serialize(std::ostream& stream, const std::multimap<Key, Value, P, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<std::pair<Key, Value>>::Serialize(stream, element);
	}

	template <typename Key, typename Value, typename P, typename Alloc>
	void BinarySerializer<std::multimap<Key, Value, P, Alloc>>::Deserialize(std::istream& stream, std::multimap<Key, Value, P, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto element = std::pair<Key, Value>{};
			BinarySerializer<std::pair<Key, Value>>::Deserialize(stream, element);
			value.insert(element);
		}
	}
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>::Serialize(std::ostream& stream, const std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<std::pair<Key, Value>>::Serialize(stream, element);
	}

	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>::Deserialize(std::istream& stream, std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto element = std::pair<Key, Value>{};
			BinarySerializer<std::pair<Key, Value>>::Deserialize(stream, element);
			value.insert(element);
		}
	}

	/** UNORDERED MULTI MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>::Serialize(std::ostream& stream, const std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value)
	{
		WriteStream(stream, value.size());

		for (auto& element : value)
			BinarySerializer<std::pair<Key, Value>>::Serialize(stream, element);
	}

	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	void BinarySerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>::Deserialize(std::istream& stream, std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value)
	{
		size_t size = ReadStream<size_t>(stream);

		value.clear();

		for (size_t i{}; i < size; ++i)
		{
			auto element = std::pair<Key, Value>{};
			BinarySerializer<std::pair<Key, Value>>::Deserialize(stream, element);
			value.insert(element);
		}
	}
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	void BinarySerializer<std::unique_ptr<T, Delete>>::Serialize(std::ostream& stream, const std::unique_ptr<T, Delete>& value)
	{
		WriteStream(stream, static_cast<bool>(value));
		if (value)
			BinarySerializer<T>::Serialize(stream, *value);
	}

	template <typename T, typename Delete>
	void BinarySerializer<std::unique_ptr<T, Delete>>::Deserialize(std::istream& stream, std::unique_ptr<T, Delete>& value)
	{
		if (ReadStream<bool>(stream))
		{
			value = std::make_unique_for_overwrite<T>();
			BinarySerializer<T>::Deserialize(stream, value);
		}
	}
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	void BinarySerializer<std::optional<T>>::Serialize(std::ostream& stream, const std::optional<T>& value)
	{
		WriteStream(stream, value.has_value());
		if (value)
			BinarySerializer<T>::Serialize(stream, value.value);
	}

	template <typename T>
	void BinarySerializer<std::optional<T>>::Deserialize(std::istream& stream, std::optional<T>& value)
	{
		if (ReadStream<bool>(stream))
		{
			auto& val = value.emplace();
			BinarySerializer<T>::Deserialize(stream, val);
		}
	}
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	void BinarySerializer<std::pair<T1, T2>>::Serialize(std::ostream& stream, const std::pair<T1, T2>& value)
	{
		BinarySerializer<T1>::Serialize(stream, value.first);
		BinarySerializer<T2>::Serialize(stream, value.second);
	}

	template <typename T1, typename T2>
	void BinarySerializer<std::pair<T1, T2>>::Deserialize(std::istream& stream, std::pair<T1, T2>& value)
	{
		BinarySerializer<T1>::Deserialize(stream, value.first);
		BinarySerializer<T2>::Deserialize(stream, value.second);
	}

	/** TUPLE */
	template <size_t Index, typename Tuple>
	void SerializeBinaryTuple(std::ostream& stream, const Tuple& tuple)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		BinarySerializer<Type>::Serialize(stream, std::get<Index>(tuple));
		if constexpr (Index + 1 < size)
		{
			SerializeBinaryTuple<Index + 1, Tuple>(stream, tuple);
		}
	}

	template <size_t Index, typename Tuple>
	void DeserializeBinaryTuple(std::istream& stream, Tuple& tuple)
	{
		using Type = std::tuple_element_t<Index, Tuple>;
		constexpr size_t size = std::tuple_size_v<Tuple>;

		BinarySerializer<Type>::Deserialize(stream, std::get<Index>(tuple));
		if constexpr (Index + 1 < size)
		{
			DeserializeBinaryTuple<Index + 1, Tuple>(stream, tuple);
		}
	}

	template <typename ... Ts>
	void BinarySerializer<std::tuple<Ts...>>::Serialize(std::ostream& stream, const std::tuple<Ts...>& value)
	{
		SerializeBinaryTuple<0>(stream, value);
	}

	template <typename ... Ts>
	void BinarySerializer<std::tuple<Ts...>>::Deserialize(std::istream& stream, std::tuple<Ts...>& value)
	{
		DeserializeBinaryTuple<0>(stream, value);
	}
#endif

#ifdef GLAS_STORAGE
	/** TYPE STORAGE */
	inline void BinarySerializer<Storage::TypeStorage>::Serialize(std::ostream& stream, const Storage::TypeStorage& value)
	{
		if (value.GetType().IsValid() && value.GetData())
		{
			WriteStream(stream, value.GetType());
			SerializeBinaryDefault(stream, value.GetData(), value.GetType());
		}
		else
		{
			WriteStream(stream, TypeId{});
		}
	}

	inline void BinarySerializer<Storage::TypeStorage>::Deserialize(std::istream& stream, Storage::TypeStorage& value)
	{
		auto type = ReadStream<TypeId>(stream);

		if (type.IsValid())
		{
			value = Storage::TypeStorage(type);
			DeserializeBinaryDefault(stream, value.GetData(), value.GetType());
		}
	}

	/** TYPE TUPLE */
	inline void BinarySerializer<Storage::TypeTuple>::Serialize(std::ostream& stream, const Storage::TypeTuple& value)
	{
		const uint32_t size{ value.GetSize() };
		auto variableIds = value.GetVariableIds();

		WriteStream(stream, size);
		stream.write(reinterpret_cast<const char*>(variableIds.data()), static_cast<std::streamsize>(sizeof(VariableId) * size));

		for (size_t i{}; i < size; ++i)
		{
			if (!variableIds[i].IsRefOrPointer())
			{
				SerializeBinaryDefault(stream, value.GetVoid(i), variableIds[i].GetTypeId());
			}
		}
	}

	inline void BinarySerializer<Storage::TypeTuple>::Deserialize(std::istream& stream, Storage::TypeTuple& value)
	{
		uint32_t size = ReadStream<uint32_t>(stream);
		
		auto variables = std::make_unique<VariableId[]>(size);
		stream.read(reinterpret_cast<char*>(variables.get()), static_cast<std::streamsize>(sizeof(VariableId) * size));

		value = Storage::TypeTuple(std::span(variables.get(), size));

		for (size_t i{}; i < size; ++i)
		{
			if (!variables[i].IsRefOrPointer())
			{
				DeserializeBinaryDefault(stream, value.GetVoid(i), variables[i].GetTypeId());
			}
		}
	}

	/** TYPE VECTOR */
	inline void BinarySerializer<Storage::TypeVector>::Serialize(std::ostream& stream, const Storage::TypeVector& value)
	{
		if (value.GetType().IsValid())
		{
			WriteStream(stream, value.GetType());
			WriteStream(stream, value.Size());

			const auto& info = value.GetType().GetInfo();
			const auto binarySerializer = info.BinarySerializer;

			for (auto element : value)
			{
				binarySerializer(stream, element);
			}
		}
		else
		{
			WriteStream(stream, TypeId{});
			WriteStream(stream, size_t{});
		}
	}

	inline void BinarySerializer<Storage::TypeVector>::Deserialize(std::istream& stream, Storage::TypeVector& value)
	{
		const TypeId id = ReadStream<TypeId>(stream);
		value = Storage::TypeVector(id);

		const size_t size = ReadStream<size_t>(stream);
		value.Resize(size);

		if (!id.IsValid())
			return;

		const auto& info = id.GetInfo();
		const auto binaryDeserializer = info.BinaryDeserializer;

		for (auto element : value)
		{
			binaryDeserializer(stream, element);
		}
	}

#endif


}

#endif