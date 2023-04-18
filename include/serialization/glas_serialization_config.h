#pragma once

#include <iostream>
#include <cassert>

#define GLAS_SERIALIZATION_STRING
#define GLAS_SERIALIZATION_VECTOR
#define GLAS_SERIALIZATION_ARRAY
#define GLAS_SERIALIZATION_DEQUE
#define GLAS_SERIALIZATION_FORWARD_LIST
#define GLAS_SERIALIZATION_LIST
#define GLAS_SERIALIZATION_SET
#define GLAS_SERIALIZATION_UNORDERED_SET
#define GLAS_SERIALIZATION_MAP
#define GLAS_SERIALIZATION_MEMORY
#define GLAS_SERIALIZATION_OPTIONAL
#define GLAS_SERIALIZATION_UTILITY

namespace glas
{
	struct TypeInfo;
}

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info);
}

/** STRING */
#if defined(GLAS_SERIALIZATION_STRING) || defined(_STRING_)
#include <string>
namespace glas::Serialization
{
	inline void Serialize(std::ostream& stream, const std::string& value);
	inline void Deserialize(std::istream& stream, std::string& value);
	inline void SerializeBinary(std::ostream& stream, const std::string& value);
	inline void DeserializeBinary(std::istream& stream, std::string& value);
}
#endif

/** VECTOR */
#if defined(GLAS_SERIALIZATION_VECTOR) || defined(_VECTOR_)
#include <vector>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::vector<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::vector<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::vector<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::vector<T>& value);
}
#endif

/** ARRAY */
#if defined(GLAS_SERIALIZATION_ARRAY) || defined(_ARRAY_)
#include <array>
namespace glas::Serialization
{
	template <typename T, size_t size>
	void Serialize(std::ostream& stream, const std::array<T, size>& value);
	template <typename T, size_t size>
	void Deserialize(std::istream& stream, std::array<T, size>& value);
	template <typename T, size_t size>
	void SerializeBinary(std::ostream& stream, const std::array<T, size>& value);
	template <typename T, size_t size>
	void DeserializeBinary(std::istream& stream, std::array<T, size>& value);
}
#endif

/** DEQUE */
#if defined(GLAS_SERIALIZATION_DEQUE) || defined(_DEQUE_)
#include <deque>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::deque<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::deque<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::deque<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::deque<T>& value);
}
#endif

/** FORWARD LIST */
#if defined(GLAS_SERIALIZATION_FORWARD_LIST) || defined(_FORWARD_LIST_)
#include <forward_list>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::forward_list<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::forward_list<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::forward_list<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::forward_list<T>& value);
}
#endif

/** LIST */
#if defined(GLAS_SERIALIZATION_LIST) || defined(_LIST_)
#include <list>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::list<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::list<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::list<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::list<T>& value);
}
#endif

/** SET & MULTI SET */
#if defined(GLAS_SERIALIZATION_SET) || defined(_SET_)
#include <set>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::set<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::set<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::set<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::set<T>& value);

	template <typename T>
	void Serialize(std::ostream& stream, const std::multiset<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::multiset<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::multiset<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::multiset<T>& value);
}
#endif

/** UNORDERED SET */
#if defined(GLAS_SERIALIZATION_UNORDERED_SET) || defined(_UNORDERED_SET_)
#include <unordered_set>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::unordered_set<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::unordered_set<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::unordered_set<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::unordered_set<T>& value);

	template <typename T>
	void Serialize(std::ostream& stream, const std::unordered_multiset<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::unordered_multiset<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::unordered_multiset<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::unordered_multiset<T>& value);
}
#endif

/** MAP */
#if defined(GLAS_SERIALIZATION_MAP) || defined(_MAP_)
#include <map>
namespace glas::Serialization
{
	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::map<Key, Value>& value);
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::map<Key, Value>& value);
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::map<Key, Value>& value);
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::map<Key, Value>& value);

	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::multimap<Key, Value>& value);
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::multimap<Key, Value>& value);
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::multimap<Key, Value>& value);
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::multimap<Key, Value>& value);
}
#endif

/** UNORDERED MAP */
#if defined(GLAS_SERIALIZATION_MAP) || defined(_UNORDERED_MAP_)
#include <unordered_map>
namespace glas::Serialization
{
	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::unordered_map<Key, Value>& value);
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::unordered_map<Key, Value>& value);
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::unordered_map<Key, Value>& value);
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::unordered_map<Key, Value>& value);

	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::unordered_multimap<Key, Value>& value);
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::unordered_multimap<Key, Value>& value);
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::unordered_multimap<Key, Value>& value);
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::unordered_multimap<Key, Value>& value);
}
#endif

/** MEMORY */
#if defined(GLAS_SERIALIZATION_MEMORY) || defined(_MEMORY_)
#include <memory>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::unique_ptr<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::unique_ptr<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::unique_ptr<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::unique_ptr<T>& value);
}
#endif

/** OPTIONAL */
#if defined(GLAS_SERIALIZATION_OPTIONAL) || defined(_OPTIONAL_)
#include <optional>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::optional<T>& value);
	template <typename T>
	void Deserialize(std::istream& stream, std::optional<T>& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::optional<T>& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::optional<T>& value);
}
#endif

/** UTILITY */
#if defined(GLAS_SERIALIZATION_UTILITY) || defined(_UTILITY_)
#include <utility>
namespace glas::Serialization
{
	template <typename T1, typename T2>
	void Serialize(std::ostream& stream, const std::pair<T1,T2>& value);
	template <typename T1, typename T2>
	void Deserialize(std::istream& stream, std::pair<T1, T2>& value);
	template <typename T1, typename T2>
	void SerializeBinary(std::ostream& stream, const std::pair<T1, T2>& value);
	template <typename T1, typename T2>
	void DeserializeBinary(std::istream& stream, std::pair<T1, T2>& value);

	template <typename... Ts>
	void Serialize(std::ostream& stream, const std::tuple<Ts...>& value);
	template <typename... Ts>
	void Deserialize(std::istream& stream, std::tuple<Ts...>& value);
	template <typename... Ts>
	void SerializeBinary(std::ostream& stream, const std::tuple<Ts...>& value);
	template <typename... Ts>
	void DeserializeBinary(std::istream& stream, std::tuple<Ts...>& value);
}
#endif

namespace glas::Serialization
{
	void Serialize(std::ostream& stream, const float& value);
	void Serialize(std::ostream& stream, const double& value);
	void Serialize(std::ostream& stream, const long double& value);

	template <typename T>
	void Serialize(std::ostream& stream, const T& value) requires std::is_fundamental_v<T>;
	template <typename T>
	void Deserialize(std::istream& stream, T& value) requires std::is_fundamental_v<T>;

	template <typename T>
	void SerializeBinary(std::ostream& stream, const T& value) requires std::is_trivially_copyable_v<T>;
	template <typename T>
	void DeserializeBinary(std::istream& stream, T& value) requires std::is_trivially_copyable_v<T>;
}

/** STORAGE */
#ifdef GLAS_STORAGE
#include "../storage/glas_storage_config.h"
namespace glas::Serialization
{
	inline void Serialize(std::ostream& stream, const Storage::TypeStorage& value);
	inline void Deserialize(std::istream& stream, Storage::TypeStorage& value);
	inline void SerializeBinary(std::ostream& stream, const Storage::TypeStorage& value);
	inline void DeserializeBinary(std::istream& stream, Storage::TypeStorage& value);

	inline void Serialize(std::ostream& stream, const Storage::TypeTuple& value);
	inline void Deserialize(std::istream& stream, Storage::TypeTuple& value);
	inline void SerializeBinary(std::ostream& stream, const Storage::TypeTuple& value);
	inline void DeserializeBinary(std::istream& stream, Storage::TypeTuple& value);
}
#endif


/** DEFAULT SERIALIZATION */
namespace glas::Serialization
{
	void Serialize(std::ostream& stream, const void* data, TypeId type);
	void Deserialize(std::istream& stream, void* data, TypeId type);
	void SerializeBinary(std::ostream& stream, const void* data, TypeId type);
	void DeserializeBinary(std::istream& stream, void* data, TypeId type);

	template <typename T>
	void Serialize(std::ostream& stream, const T& value);
	template <typename T>
	void Deserialize(std::istream& stream, T& value);
	template <typename T>
	void SerializeBinary(std::ostream& stream, const T& value);
	template <typename T>
	void DeserializeBinary(std::istream& stream, T& value);
}