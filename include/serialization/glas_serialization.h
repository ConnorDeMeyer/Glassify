#pragma once

#include "glas_serialization_config.h"
#include "../glas_config.h"

#include <type_traits>
#include <sstream>

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info)
	{
		info.Serializer = [](std::ostream& stream, const void* data) { Serialize(stream, *static_cast<const T*>(data)); };
		info.Deserializer = [](std::istream& stream, void* data) { Deserialize(stream, *static_cast<T*>(data)); };
		info.BinarySerializer = [](std::ostream& stream, const void* data) { SerializeBinary(stream, *static_cast<const T*>(data)); };
		info.BinaryDeserializer = [](std::istream& stream, void* data) { DeserializeBinary(stream, *static_cast<T*>(data)); };
	}

	/** HELPER FUNCTIONS */

	inline const void* VoidOffset(const void* data, size_t offset)
	{
		return static_cast<const uint8_t*>(data) + offset;
	}

	inline void* VoidOffset(void* data, size_t offset)
	{
		return static_cast<uint8_t*>(data) + offset;
	}

	template <typename T>
	void WriteStream(std::ostream& stream, const T& value)
	{
		stream.write(reinterpret_cast<const char*>(value), sizeof(T));
	}

	template <typename T>
	T ReadStream(std::istream& stream)
	{
		T t;
		stream.read(reinterpret_cast<char*>(t), sizeof(T));
		return t;
	}

	inline void IStreamChar(std::istream& stream, char expectedChar)
	{
		char buffer{};
		stream >> buffer;
		assert(expectedChar == buffer);
	}
}

/** STRING */
#if defined(GLAS_SERIALIZATION_STRING) || defined(_STRING_)
namespace glas::Serialization
{
	inline void Serialize(std::ostream& stream, const std::string& value)
	{
		stream << '\"' << value << '\"';
	}

	inline void Deserialize(std::istream& stream, std::string& value)
	{
		value.clear();
		char buffer{};

		IStreamChar(stream, '\"');

		stream >> buffer;

		while (buffer != '"')
		{
			value += buffer;
			stream >> buffer;
		}
	}

	inline void SerializeBinary(std::ostream& stream, const std::string& value)
	{
		size_t size = value.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
		stream.write(value.data(), size);
	}

	inline void DeserializeBinary(std::istream& stream, std::string& value)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		value.clear();
		value.resize(size);

		stream.read(value.data(), size);
	}
}
#endif

/** CONTAINER HELPERS */
#pragma region ContainerHelpers
namespace glas::Serialization
{
	template <typename Container>
	void SerializeContainer(std::ostream& stream, const Container& container)
	{
		stream << '[';

		int counter{};
		for (auto& val : container)
		{
			if (counter++ != 0)
				stream << ',';

			Serialize(stream, val);
		}

		stream << ']';
	}

	template <typename Container>
	void SerializeMapContainer(std::ostream& stream, const Container& container)
	{
		stream << '{';

		int counter{};
		for (auto& [key, val] : container)
		{
			if (counter++ != 0)
				stream << ',';

			Serialize(stream, key);
			stream << ": ";
			Serialize(stream, val);
		}

		stream << '}';
	}

	template <typename Container, typename ContainerType>
	void DeserializeContainer(std::istream& stream, Container& container, const std::function<void(Container&, ContainerType&&)>& addingFunction)
	{
		char buffer{};
		IStreamChar(stream, '[');

		while (buffer != ']')
		{
			typename Container::value_type entry{};
			Deserialize(stream, entry);
			addingFunction(container, std::move(entry));

			stream >> buffer;
		}
	}

	template <typename Container>
	void DeserializeMapContainer(std::istream& stream, Container& container)
	{
		char buffer{};
		IStreamChar(stream, '{');

		while (buffer != '}')
		{
			std::remove_const_t<typename Container::key_type> key{};
			Deserialize(stream, key);

			stream >> buffer;
			assert(buffer == ':');

			Deserialize(stream, container.emplace(std::move(key), std::move(typename Container::value_type::second_type{})).first->second);

			stream >> buffer;
		}
	}

	template <typename Container>
	void SerializeBinaryContainer(std::ostream& stream, const Container& container)
	{
		size_t size = container.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

		for (auto& entry : container)
		{
			SerializeBinary(stream, entry);
		}
	}

	template <typename Container>
	void SerializeBinaryMapContainer(std::ostream& stream, const Container& container)
	{
		size_t size = container.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

		for (auto& [key, value] : container)
		{
			SerializeBinary(stream, key);
			SerializeBinary(stream, value);
		}
	}

	template <typename Container, typename ContainerType>
	void DeserializeBinaryContainer(std::istream& stream, Container& container, const std::function<void(Container&, ContainerType&&)>& addingFunction)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		for (size_t i{}; i < size; ++i)
		{
			typename Container::value_type entry{};
			DeserializeBinary(stream, entry);
			addingFunction(container, std::move(entry));
		}
	}

	template <typename Container>
	void DeserializeBinaryMapContainer(std::istream& stream, Container& container)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		for (size_t i{}; i < size; ++i)
		{
			std::remove_const_t<typename Container::value_type::first_type> key{};
			typename Container::value_type::second_type value{};
			DeserializeBinary(stream, key);
			DeserializeBinary(stream, value);
			container.emplace(std::move(key), std::move(value));
		}
	}
}
#pragma endregion

/** VECTOR */
#if defined(GLAS_SERIALIZATION_VECTOR) || defined(_VECTOR_)
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::vector<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::vector<T>& value)
	{
		std::function<void(std::vector<T>&, T&&)> addingFunction = [](std::vector<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::vector<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::vector<T>& value)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		value.reserve(size);

		for (size_t i{}; i < size; ++i)
		{
			T entry{};
			DeserializeBinary(stream, entry);
			value.emplace_back(std::move(entry));
		}
	}
}
#endif

/** ARRAY */
#if defined(GLAS_SERIALIZATION_ARRAY) || defined(_ARRAY_)
namespace glas::Serialization
{
	template <typename T, size_t size>
	void Serialize(std::ostream& stream, const std::array<T, size>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T, size_t size>
	void Deserialize(std::istream& stream, std::array<T, size>& value)
	{
		char buffer{};
		IStreamChar(stream, '[');

		size_t counter{};
		while (buffer != ']')
		{
			Deserialize(stream, value[counter]);

			stream >> buffer;
			++counter;
		}
	}
	template <typename T, size_t size>
	void SerializeBinary(std::ostream& stream, const std::array<T, size>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T, size_t size>
	void DeserializeBinary(std::istream& stream, std::array<T, size>& value)
	{
		size_t arraySize{};
		stream.read(reinterpret_cast<char*>(&arraySize), sizeof(size_t));
		assert(arraySize == size);

		for (size_t i{}; i < size; ++i)
		{
			DeserializeBinary(stream, value[i]);
		}
	}
}
#endif

/** DEQUE */
#if defined(GLAS_SERIALIZATION_DEQUE) || defined(_DEQUE_)
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::deque<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::deque<T>& value)
	{
		std::function<void(std::deque<T>&, T&&)> addingFunction = [](std::deque<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::deque<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::deque<T>& value)
	{
		std::function<void(std::deque<T>&, T&&)> addingFunction = [](std::deque<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}
}
#endif

/** FORWARD LIST */
#if defined(GLAS_SERIALIZATION_FORWARD_LIST) || defined(_FORWARD_LIST_)
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::forward_list<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::forward_list<T>& value)
	{
		std::function<void(std::forward_list<T>&, T&&)> addingFunction = [](std::forward_list<T>& container, T&& val) { container.emplace_front(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::forward_list<T>& value)
	{
		size_t size = std::distance(value.begin(), value.end());
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

		for (auto& entry : value)
		{
			SerializeBinary(stream, entry);
		}
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::forward_list<T>& value)
	{
		std::function<void(std::forward_list<T>&, T&&)> addingFunction = [](std::forward_list<T>& container, T&& val) { container.emplace_front(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}
}
#endif

/** LIST */
#if defined(GLAS_SERIALIZATION_LIST) || defined(_LIST_)
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::list<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::list<T>& value)
	{
		std::function<void(std::list<T>&, T&&)> addingFunction = [](std::list<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::list<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::list<T>& value)
	{
		std::function<void(std::list<T>&, T&&)> addingFunction = [](std::list<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}
}
#endif

/** SET & MULTI SET */
#if defined(GLAS_SERIALIZATION_SET) || defined(_SET_)
#include <set>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::set<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::set<T>& value)
	{
		std::function<void(std::set<T>&, T&&)> addingFunction = [](std::set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::set<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::set<T>& value)
	{
		std::function<void(std::set<T>&, T&&)> addingFunction = [](std::set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	template <typename T>
	void Serialize(std::ostream& stream, const std::multiset<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::multiset<T>& value)
	{
		std::function<void(std::multiset<T>&, T&&)> addingFunction = [](std::multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::multiset<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::multiset<T>& value)
	{
		std::function<void(std::multiset<T>&, T&&)> addingFunction = [](std::multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}
}
#endif

/** UNORDERED SET */
#if defined(GLAS_SERIALIZATION_UNORDERED_SET) || defined(_UNORDERED_SET_)
#include <unordered_set>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::unordered_set<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::unordered_set<T>& value)
	{
		std::function<void(std::unordered_set<T>&, T&&)> addingFunction = [](std::unordered_set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::unordered_set<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::unordered_set<T>& value)
	{
		std::function<void(std::unordered_set<T>&, T&&)> addingFunction = [](std::unordered_set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	template <typename T>
	void Serialize(std::ostream& stream, const std::unordered_multiset<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	void Deserialize(std::istream& stream, std::unordered_multiset<T>& value)
	{
		std::function<void(std::unordered_multiset<T>&, T&&)> addingFunction = [](std::unordered_multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::unordered_multiset<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	void DeserializeBinary(std::istream& stream, std::unordered_multiset<T>& value)
	{
		std::function<void(std::unordered_multiset<T>&, T&&)> addingFunction = [](std::unordered_multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}
}
#endif

/** MAP */
#if defined(GLAS_SERIALIZATION_MAP) || defined(_MAP_)
#include <map>
namespace glas::Serialization
{
	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::map<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::map<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::map<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::map<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}

	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::multimap<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::multimap<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::multimap<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::multimap<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}
}
#endif

/** UNORDERED MAP */
#if defined(GLAS_SERIALIZATION_MAP) || defined(_UNORDERED_MAP_)
#include <unordered_map>
namespace glas::Serialization
{
	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::unordered_map<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::unordered_map<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::unordered_map<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::unordered_map<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}

	template <typename Key, typename Value>
	void Serialize(std::ostream& stream, const std::unordered_multimap<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void Deserialize(std::istream& stream, std::unordered_multimap<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void SerializeBinary(std::ostream& stream, const std::unordered_multimap<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	void DeserializeBinary(std::istream& stream, std::unordered_multimap<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}
}
#endif

/** MEMORY */
#if defined(GLAS_SERIALIZATION_MEMORY) || defined(_MEMORY_)
#include <memory>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::unique_ptr<T>& value)
	{
		stream << static_cast<bool>(value);
		if (value)
		{
			stream << ": ";
			Serialize(stream, *value);
		}
	}

	template <typename T>
	void Deserialize(std::istream& stream, std::unique_ptr<T>& value)
	{
		bool hasValue{};
		stream >> hasValue;
		if (hasValue)
		{
			IStreamChar(stream, ':');

			value = std::make_unique<T>();
			Deserialize(stream, *value);
		}
	}

	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::unique_ptr<T>& value)
	{
		bool hasValue{ value };
		stream.write(reinterpret_cast<const char*>(&hasValue), sizeof(bool));
		if (hasValue)
		{
			SerializeBinary(stream, *value);
		}
	}

	template <typename T>
	void DeserializeBinary(std::istream& stream, std::unique_ptr<T>& value)
	{
		bool hasValue{};
		stream.read(reinterpret_cast<char*>(&hasValue), sizeof(bool));
		if (hasValue)
		{
			value = std::make_unique<T>();
			DeserializeBinary(stream, *value);
		}
	}
}
#endif

/** OPTIONAL */
#if defined(GLAS_SERIALIZATION_OPTIONAL) || defined(_OPTIONAL_)
#include <optional>
namespace glas::Serialization
{
	template <typename T>
	void Serialize(std::ostream& stream, const std::optional<T>& value)
	{
		stream << value.has_value();
		if (value.has_value())
		{
			stream << ": ";
			Serialize(stream, *value);
		}
	}

	template <typename T>
	void Deserialize(std::istream& stream, std::optional<T>& value)
	{
		bool hasValue{};
		stream >> hasValue;
		if (hasValue)
		{
			IStreamChar(stream, ':');

			value.emplace();
			Deserialize(stream, *value);
		}
	}

	template <typename T>
	void SerializeBinary(std::ostream& stream, const std::optional<T>& value)
	{
		bool hasValue{ value.has_value() };
		stream.write(reinterpret_cast<const char*>(&hasValue), sizeof(bool));
		if (hasValue)
		{
			SerializeBinary(stream, *value);
		}
	}

	template <typename T>
	void DeserializeBinary(std::istream& stream, std::optional<T>& value)
	{
		bool hasValue{};
		stream.read(reinterpret_cast<char*>(&hasValue), sizeof(bool));
		if (hasValue)
		{
			value.emplace();
			DeserializeBinary(stream, *value);
		}
	}
}
#endif

/** UTILITY */
#if defined(GLAS_SERIALIZATION_UTILITY) || defined(_UTILITY_)
namespace glas::Serialization
{
	template <typename T1, typename T2>
	void Serialize(std::ostream& stream, const std::pair<T1, T2>& value)
	{
		stream << '{';
		Serialize(stream, value.first);
		stream << ',';
		Serialize(stream, value.second);
		stream << '}';
	}

	template <typename T1, typename T2>
	void Deserialize(std::istream& stream, std::pair<T1, T2>& value)
	{
		IStreamChar(stream, '{');

		Deserialize(stream, value.first);

		IStreamChar(stream, ':');

		Deserialize(stream, value.second);

		IStreamChar(stream, '}');
	}

	template <typename T1, typename T2>
	void SerializeBinary(std::ostream& stream, const std::pair<T1, T2>& value)
	{
		SerializeBinary(stream, value.first);
		SerializeBinary(stream, value.second);
	}

	template <typename T1, typename T2>
	void DeserializeBinary(std::istream& stream, std::pair<T1, T2>& value)
	{
		DeserializeBinary(stream, value.first);
		DeserializeBinary(stream, value.second);
	}

	template <size_t Index, typename... Ts>
	void Serialize(std::ostream& stream, const std::tuple<Ts...>& value)
	{
		Serialize(stream, std::get<Index>(value));
		if constexpr (Index + 1 < sizeof...(Ts))
		{
			stream << ',';
			Serialize<Index + 1, Ts...>(stream, value);
		}
	}

	template <typename... Ts>
	void Serialize(std::ostream& stream, const std::tuple<Ts...>& value)
	{
		stream << '{';
		Serialize<0, Ts...>(stream, value);
		stream << '}';
	}

	template <size_t Index, typename... Ts>
	void Deserialize(std::istream& stream, std::tuple<Ts...>& value)
	{
		Deserialize(stream, std::get<Index>(value));
		if constexpr (Index + 1 < sizeof...(Ts))
		{
			IStreamChar(stream, ',');

			Deserialize<Index + 1, Ts...>(stream, value);
		}
	}

	template <typename... Ts>
	void Deserialize(std::istream& stream, std::tuple<Ts...>& value)
	{
		IStreamChar(stream, '{');

		Deserialize<0, Ts...>(stream, value);

		IStreamChar(stream, '}');
	}

	template <size_t Index, typename... Ts>
	void SerializeBinary(std::ostream& stream, const std::tuple<Ts...>& value)
	{
		SerializeBinary(stream, std::get<Index>(value));
		if constexpr (Index + 1 < sizeof...(Ts))
		{
			SerializeBinary<Index + 1, Ts...>(stream, value);
		}
	}

	template <typename... Ts>
	void SerializeBinary(std::ostream& stream, const std::tuple<Ts...>& value)
	{
		SerializeBinary<0, Ts...>(stream, value);
	}

	template <size_t Index, typename... Ts>
	void DeserializeBinary(std::istream& stream, std::tuple<Ts...>& value)
	{
		DeserializeBinary(stream, std::get<Index>(value));
		if constexpr (Index + 1 < sizeof...(Ts))
		{
			DeserializeBinary<Index + 1, Ts...>(stream, value);
		}
	}

	template <typename... Ts>
	void DeserializeBinary(std::istream& stream, std::tuple<Ts...>& value)
	{
		DeserializeBinary<0, Ts...>(stream, value);
	}
}
#endif

namespace glas::Serialization
{
	void Serialize(std::ostream& stream, const float& value)
	{
		if (std::isnan(value) || std::isinf(value))
			throw std::runtime_error("float value is invalid");
		stream << value;
	}

	void Serialize(std::ostream& stream, const double& value)
	{
		if (std::isnan(value) || std::isinf(value))
			throw std::runtime_error("float value is invalid");
		stream << value;
	}

	void Serialize(std::ostream& stream, const long double& value)
	{
		if (std::isnan(value) || std::isinf(value))
			throw std::runtime_error("float value is invalid");
		stream << value;
	}

	/** FUNDAMENTAL TYPES*/

	template <typename T>
	void Serialize(std::ostream& stream, const T& value) requires std::is_fundamental_v<T>
	{
		stream << value;
	}

	template <typename T>
	void Deserialize(std::istream& stream, T& value) requires std::is_fundamental_v<T>
	{
		stream >> value;
	}

	/** TRIVIALLY COPYABLE*/

	template <typename T>
	void SerializeBinary(std::ostream& stream, const T& value) requires std::is_trivially_copyable_v<T>
	{
		stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
	}

	template <typename T>
	void DeserializeBinary(std::istream& stream, T& value) requires std::is_trivially_copyable_v<T>
	{
		stream.read(reinterpret_cast<char*>(&value), sizeof(T));
	}
}

#ifdef GLAS_STORAGE
namespace glas::Serialization
{
	inline void Serialize(std::ostream& stream, const Storage::TypeStorage& value)
	{
		stream << value.GetType().GetId();
		if (value.GetData())
		{
			Serialize(stream, value.GetData(), value.GetType());
		}
	}

	inline void Deserialize(std::istream& stream, Storage::TypeStorage& value)
	{
		uint64_t id{};
		stream >> id;
		TypeId typeId{ id };

		if (id)
		{
			value = Storage::TypeStorage(typeId);
			Deserialize(stream, value.GetData(), typeId);
		}
	}

	inline void SerializeBinary(std::ostream& stream, const Storage::TypeStorage& value)
	{
		TypeId type = value.GetType();
		stream.write(reinterpret_cast<const char*>(&type), sizeof(TypeId));
		if (value.GetData())
		{
			SerializeBinary(stream, value.GetData(), value.GetType());
		}
	}

	inline void DeserializeBinary(std::istream& stream, Storage::TypeStorage& value)
	{
		TypeId typeId{ };
		stream.read(reinterpret_cast<char*>(&typeId), sizeof(TypeId));

		if (typeId.IsValid())
		{
			value = Storage::TypeStorage(typeId);
			DeserializeBinary(stream, value.GetData(), typeId);
		}
	}

	inline void Serialize(std::ostream& stream, const Storage::TypeTuple& value)
	{
		stream << "{ ";

		stream << value.GetSize();

		stream << " {";
		const auto variableIds = value.GetVariableIds();

		for (size_t i{}; i < variableIds.size(); ++i)
		{
			if (i != 0)
				stream << ',';

			stream << variableIds[i];
		}

		stream << " },{";

		size_t variableStreamed{};
		for (size_t i{}; i < variableIds.size(); ++i)
		{
			if (!variableIds[i].IsRefOrPointer())
			{
				if (variableStreamed++ != 0)
					stream << ',';

				Serialize(stream, value.GetVoid(i), variableIds[i].GetTypeId());
			}
		}

		stream << " }}";
	}

	inline void Deserialize(std::istream& stream, Storage::TypeTuple& value)
	{
		IStreamChar(stream, '{');

		uint32_t size{};
		stream >> size;

		IStreamChar(stream, '{');

		std::vector<VariableId> variableIds{};
		variableIds.reserve(size);

		for (size_t i{}; i < size; ++i)
		{
			VariableId id{};
			stream >> id;
			variableIds.push_back(id);
		}

		IStreamChar(stream, '}');
		IStreamChar(stream, ',');
		IStreamChar(stream, '{');

		value = Storage::TypeTuple(std::span<VariableId>{variableIds.begin(), variableIds.end()});

		bool parsedVariable{};
		for (size_t i{}; i < size; ++i)
		{
			if (!variableIds[i].IsRefOrPointer())
			{
				if (!parsedVariable)
				{
					IStreamChar(stream, ',');
				}
				parsedVariable = true;

				Deserialize(stream, value.GetVoid(i), variableIds[i].GetTypeId());
			}
		}

		IStreamChar(stream, '}');
		IStreamChar(stream, '}');
	}

	inline void SerializeBinary(std::ostream& stream, const Storage::TypeTuple& value)
	{
		uint32_t size{ value.GetSize() };
		auto variableIds = value.GetVariableIds();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
		stream.write(reinterpret_cast<const char*>(variableIds.data()), sizeof(VariableId) * size);

		for (size_t i{}; i < size; ++i)
		{
			if (!variableIds[i].IsRefOrPointer())
			{
				SerializeBinary(stream, value.GetVoid(i), variableIds[i].GetTypeId());
			}
		}
	}

	inline void DeserializeBinary(std::istream& stream, Storage::TypeTuple& value)
	{
		uint32_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));

		auto variables = std::make_unique<VariableId[]>(size);
		stream.read(reinterpret_cast<char*>(variables.get()), sizeof(VariableId) * size);

		value = Storage::TypeTuple(std::span(variables.get(), size));

		for (size_t i{}; i < size; ++i)
		{
			if (!variables[i].IsRefOrPointer())
			{
				DeserializeBinary(stream, value.GetVoid(i), variables[i].GetTypeId());
			}
		}
	}
#endif

}

/** DEFAULT SERIALIZATION */
namespace glas::Serialization
{
	void Serialize(std::ostream& stream, const void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		stream << '{';
		int memberOutputCount{};

		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!member.VariableId.IsRefOrPointer())
			{
				if (memberOutputCount++ != 0)
					stream << ',';

				stream << '\"' << member.Name << "\": ";
				member.VariableId.GetTypeId().GetInfo().Serializer(stream, VoidOffset(data, member.Offset));
			}
		}
		stream << '}';
	}

	void Deserialize(std::istream& stream, void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		std::string MemberName{};
		MemberName.reserve(64);

		char buffer{};
		IStreamChar(stream, '{');

		auto& members = info.Members;
		while (buffer != '}')
		{
			Deserialize(stream, MemberName);

			auto member = std::find_if(members.begin(), members.end(), [&MemberName](const MemberInfo& info) {return info.Name == MemberName; });
			if (member != members.end() && !member->VariableId.IsRefOrPointer())
			{
				IStreamChar(stream, ':');
				member->VariableId.GetTypeId().GetInfo().Deserializer(stream, VoidOffset(data, member->Offset));
			}

			stream >> buffer;
		}
	}

	void SerializeBinary(std::ostream& stream, const void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!member.VariableId.IsRefOrPointer())
			{
				member.VariableId.GetTypeId().GetInfo().BinarySerializer(stream, VoidOffset(data, member.Offset));
			}
		}
	}

	void DeserializeBinary(std::istream& stream, void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		auto& members = info.Members;
		for (auto& member : members)
		{
			if (!member.VariableId.IsRefOrPointer())
			{
				member.VariableId.GetTypeId().GetInfo().BinaryDeserializer(stream, VoidOffset(data, member.Offset));
			}
		}
	}

	template <typename T>
	void Serialize(std::ostream& stream, const T& value)
	{
		Serialize(stream, &value, TypeId::Create<T>());
	}

	template <typename T>
	void Deserialize(std::istream& stream, T& value)
	{
		Deserialize(stream, &value, TypeId::Create<T>());
	}

	template <typename T>
	void SerializeBinary(std::ostream& stream, const T& value)
	{
		SerializeBinary(stream, &value, TypeId::Create<T>());
	}

	template <typename T>
	void DeserializeBinary(std::istream& stream, T& value)
	{
		DeserializeBinary(stream, &value, TypeId::Create<T>());
	}
}