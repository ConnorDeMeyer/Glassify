#pragma once

#include <iostream>
#include <cassert>

#include <vector>
#include <array>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <list>
#include <deque>
#include <forward_list>

namespace glas
{
	struct TypeInfo;
}

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info);

	inline void Serialize(std::ostream& stream, const std::string& value);
	inline void Deserialize(std::istream& stream, std::string& value);
	inline void SerializeBinary(std::ostream& stream, const std::string& value);
	inline void DeserializeBinary(std::istream& stream, std::string& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::vector<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::vector<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::vector<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::vector<T>& value);

	template <typename T, size_t size>
	inline void Serialize(std::ostream& stream, const std::array<T, size>& value);
	template <typename T, size_t size>
	inline void Deserialize(std::istream& stream, std::array<T, size>& value);
	template <typename T, size_t size>
	inline void SerializeBinary(std::ostream& stream, const std::array<T, size>& value);
	template <typename T, size_t size>
	inline void DeserializeBinary(std::istream& stream, std::array<T, size>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::deque<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::deque<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::deque<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::deque<T>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::forward_list<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::forward_list<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::forward_list<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::forward_list<T>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::list<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::list<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::list<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::list<T>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::set<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::set<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::set<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::set<T>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::multiset<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::multiset<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::multiset<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::multiset<T>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::unordered_set<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::unordered_set<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_set<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::unordered_set<T>& value);

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::unordered_multiset<T>& value);
	template <typename T>
	inline void Deserialize(std::istream& stream, std::unordered_multiset<T>& value);
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_multiset<T>& value);
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::unordered_multiset<T>& value);

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::map<Key, Value>& value);
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::map<Key, Value>& value);
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::map<Key, Value>& value);
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::map<Key, Value>& value);

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::multimap<Key, Value>& value);
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::multimap<Key, Value>& value);
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::multimap<Key, Value>& value);
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::multimap<Key, Value>& value);

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::unordered_map<Key, Value>& value);
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::unordered_map<Key, Value>& value);
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_map<Key, Value>& value);
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::unordered_map<Key, Value>& value);

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::unordered_multimap<Key, Value>& value);
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::unordered_multimap<Key, Value>& value);
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_multimap<Key, Value>& value);
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::unordered_multimap<Key, Value>& value);


	/** CONCEPTS */

	template <typename T>
	concept OutStreamable = requires (std::ostream str, T val) { str << val; };

	template <typename T>
	concept InStreamable = requires (std::istream str, T val) { str >> val; };

	template <typename T>
	concept Streamable = OutStreamable<T> && InStreamable<T>;

	template <typename T>
	concept OutSerializable = requires(T t, std::ostream stream) { Serialize(stream, t); };

	template <typename T>
	concept InSerializable = requires(T t, std::istream stream) { Deserialize(stream, t); };

	template <typename T>
	concept Serializable = InSerializable<T> && OutSerializable<T>;

	template <typename T>
	concept InSerializableBinary = requires(T t, std::ostream stream) { SerializeBinary(stream, t); };

	template <typename T>
	concept OutSerializableBinary = requires(T t, std::istream stream) { DeserializeBinary(stream, t); };

	template <typename T>
	concept SerializableBinary = InSerializableBinary<T> && OutSerializableBinary<T>;
}