#pragma once

#ifdef GLAS_SERIALIZATION_JSON

#include <cassert>
#include <sstream>
#include <type_traits>

/** Optional includes*/
#include <string>
#include <vector>
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <utility>

#include "../glas_decl.h"

#ifdef GLAS_STORAGE
#include "../storage/glas_storage_config.h"
#endif

namespace glas
{
	struct TypeInfo;
}

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoJSon(TypeInfo& info);
}

namespace glas::Serialization
{
	using RapidJsonAllocator = RAPIDJSON_DEFAULT_ALLOCATOR;

	template <typename T>
	struct JSonSerializer;
}

namespace glas::Serialization
{
	void SerializeJSon(std::ostream& stream, const void* data, TypeId type);
	void DeserializeJSon(std::istream& stream, void* data, TypeId type);

	template <typename T>
	void SerializeJSon(std::ostream& stream, const T& value);
	template <typename T>
	void DeserializeJSon(std::istream& stream, T& value);

	void SerializeJSonDefault(rapidjson::Value& jsonVal, const void* data, TypeId type, RapidJsonAllocator& allocator);
	void DeserializeJSonDefault(rapidjson::Value& jsonVal, void* data, TypeId type);

	/** DEFAULT SERIALIZATION */
	template <typename T>
	struct JSonSerializer
	{
		static void Serialize(rapidjson::Value& jsonVal, const T& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, T& value);
	};

	/** FLOAT */
	template <>
	struct JSonSerializer<float>
	{
		static void Serialize(rapidjson::Value& jsonVal, const float& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, float& value);
	};

	/** DOUBLE */
	template <>
	struct JSonSerializer<double>
	{
		static void Serialize(rapidjson::Value& jsonVal, const double& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, double& value);
	};

	/** INT */
	template <>
	struct JSonSerializer<int>
	{
		static void Serialize(rapidjson::Value& jsonVal, const int& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, int& value);
	};

	/** INT 64 */
	template <>
	struct JSonSerializer<int64_t>
	{
		static void Serialize(rapidjson::Value& jsonVal, const int64_t& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, int64_t& value);
	};

	/** UINT */
	template <>
	struct JSonSerializer<unsigned int>
	{
		static void Serialize(rapidjson::Value& jsonVal, const unsigned int& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, unsigned int& value);
	};

	/** UINT 64 */
	template <>
	struct JSonSerializer<uint64_t>
	{
		static void Serialize(rapidjson::Value& jsonVal, const uint64_t& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, uint64_t& value);
	};

	/** BOOL */
	template <>
	struct JSonSerializer<bool>
	{
		static void Serialize(rapidjson::Value& jsonVal, const bool& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, bool& value);
	};

	/** STRING */
	template <>
	struct JSonSerializer<char*>
	{
		static void Serialize(rapidjson::Value& jsonVal, const char*& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, const char*& value);
	};

	/** ARRAY */
	template <typename T>
	struct JSonSerializer<T[]>
	{
		static void Serialize(rapidjson::Value& jsonVal, const T value[], RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, T value[]);
	};

	template <>
	struct JSonSerializer<TypeId>
	{
		static void Serialize(rapidjson::Value& jsonVal, const TypeId& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, TypeId& value);
	};

	template <>
	struct JSonSerializer<VariableId>
	{
		static void Serialize(rapidjson::Value& jsonVal, const VariableId& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, VariableId& value);
	};

	template <>
	struct JSonSerializer<FunctionId>
	{
		static void Serialize(rapidjson::Value& jsonVal, const FunctionId& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, FunctionId& value);
	};

#ifdef _STRING_
	/** STRING */
	template <typename Elem, typename Traits, typename Alloc>
	struct JSonSerializer<std::basic_string<Elem, Traits, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::basic_string<Elem, Traits, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::basic_string<Elem, Traits, Alloc>& value);
	};
#endif

#ifdef _VECTOR_
	/** VECTOR */
	template <typename T, typename Alloc>
	struct JSonSerializer<std::vector<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::vector<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::vector<T, Alloc>& value);
	};
#endif

#ifdef _ARRAY_
	/** ARRAY */
	template <typename T, size_t Size>
	struct JSonSerializer<std::array<T, Size>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::array<T, Size>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::array<T, Size>& value);
	};
#endif

#ifdef _DEQUE_
	/** DEQUE*/
	template <typename T, typename Alloc>
	struct JSonSerializer<std::deque<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::deque<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::deque<T, Alloc>& value);
	};
#endif

#ifdef _FORWARD_LIST_
	/** FORWARD LIST */
	template <typename T, typename Alloc>
	struct JSonSerializer<std::forward_list<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::forward_list<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::forward_list<T, Alloc>& value);
	};
#endif

#ifdef _LIST_
	/** LIST */
	template <typename T, typename Alloc>
	struct JSonSerializer<std::list<T, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::list<T, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::list<T, Alloc>& value);
	};
#endif

#ifdef _SET_
	/** UNORDERED SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_set<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_set<T, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_set<T, Hasher, Keyeq, Alloc>& value);
	};

	/** UNORDERED MULTI SET */
	template <typename T, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_multiset<T, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_multiset<T, Hasher, Keyeq, Alloc>& value);
	};
#endif

#ifdef _MAP_
	/** MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	struct JSonSerializer<std::map<Key, Value, P, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::map<Key, Value, P, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::map<Key, Value, P, Alloc>& value);
	};

	/** MULTI MAP */
	template <typename Key, typename Value, typename P, typename Alloc>
	struct JSonSerializer<std::multimap<Key, Value, P, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::multimap<Key, Value, P, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::multimap<Key, Value, P, Alloc>& value);
	};
#endif

#ifdef _UNORDERED_MAP_
	/** UNORDERED MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_map<Key, Value, Hasher, Keyeq, Alloc>& value);
	};

	/** UNORDERED MULTI MAP */
	template <typename Key, typename Value, typename Hasher, typename Keyeq, typename Alloc>
	struct JSonSerializer<std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unordered_multimap<Key, Value, Hasher, Keyeq, Alloc>& value);
	};
#endif

#ifdef _MEMORY_
	/** UNIQUE PTR */
	template <typename T, typename Delete>
	struct JSonSerializer<std::unique_ptr<T, Delete>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::unique_ptr<T, Delete>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::unique_ptr<T, Delete>& value);
	};
#endif

#ifdef _OPTIONAL_
	/** OPTIONAL */
	template <typename T>
	struct JSonSerializer<std::optional<T>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::optional<T>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::optional<T>& value);
	};
#endif

#ifdef _UTILITY_
	/** PAIR */
	template <typename T1, typename T2>
	struct JSonSerializer<std::pair<T1, T2>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::pair<T1, T2>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::pair<T1, T2>& value);
	};

	/** TUPLE */
	template <typename ... Ts>
	struct JSonSerializer<std::tuple<Ts...>>
	{
		static void Serialize(rapidjson::Value& jsonVal, const std::tuple<Ts...>& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, std::tuple<Ts...>& value);
	};
#endif

#ifdef GLAS_STORAGE
	/** TYPE STORAGE */
	template <>
	struct JSonSerializer<Storage::TypeStorage>
	{
		static void Serialize(rapidjson::Value& jsonVal, const Storage::TypeStorage& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, Storage::TypeStorage& value);
	};

	/** TYPE TUPLE */
	template <>
	struct JSonSerializer<Storage::TypeTuple>
	{
		static void Serialize(rapidjson::Value& jsonVal, const Storage::TypeTuple& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, Storage::TypeTuple& value);
	};

	/** TYPE VECTOR */
	template <>
	struct JSonSerializer<Storage::TypeVector>
	{
		static void Serialize(rapidjson::Value& jsonVal, const Storage::TypeVector& value, RapidJsonAllocator& allocator);
		static void Deserialize(rapidjson::Value& jsonVal, Storage::TypeVector& value);
	};
#endif
}

#endif