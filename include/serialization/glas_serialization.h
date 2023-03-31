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
		if constexpr (Serializable<T>)
		{
			info.Serializer = [](std::ostream& stream, const void* data) { Serialize(stream, *static_cast<const T*>(data)); };
			info.Deserializer = [](std::istream& stream, void* data) { Deserialize(stream, *static_cast<T*>(data)); };
		}
		else if constexpr (Streamable<T>)
		{
			info.Serializer = [](std::ostream& stream, const void* data) { stream << *static_cast<const T*>(data); };
			info.Deserializer = [](std::istream& stream, void* data) { stream >> *static_cast<T*>(data); };
		}
		
		if constexpr (SerializableBinary<T>)
		{
			info.BinarySerializer = [](std::ostream& stream, const void* data) { SerializeBinary(stream, *static_cast<const T*>(data)); };
			info.BinaryDeserializer = [](std::istream& stream, void* data) { DeserializeBinary(stream, *static_cast<T*>(data)); };
		}
		else if constexpr (std::is_trivially_copyable_v<T>)
		{
			info.BinarySerializer = [](std::ostream& stream, const void* data) { stream.write(reinterpret_cast<const char*>(data), sizeof(T)); };
			info.BinaryDeserializer = [](std::istream& stream, void* data) { stream.read(reinterpret_cast<char*>(data), sizeof(T)); };
		}
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

	/** SERIALIZER */

	inline void SerializeType(std::ostream& stream, const void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		if (info.Serializer)
		{
			info.Serializer(stream, data);
		}
		else
		{
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
					SerializeType(stream, VoidOffset(data, member.Offset), member.VariableId.GetTypeId());
				}
			}
			stream << '}';
		}
	}

	template <typename T>
	void SerializeType(std::ostream& stream, const T& data)
	{
		SerializeType(stream, &data, TypeId::Create<std::remove_cvref_t<T>>());
	}

	inline void SerializeTypeBinary(std::ostream& stream, const void* data, TypeId type)
	{
		auto& info = GetTypeInfo(type);

		if (info.BinarySerializer)
		{
			info.BinarySerializer(stream, data);
		}
		else
		{
			auto& members = info.Members;
			for (auto& member : members)
			{
				if (!member.VariableId.IsRefOrPointer())
				{
					SerializeTypeBinary(stream, VoidOffset(data, member.Offset), member.VariableId.GetTypeId());
				}
			}
		}
	}

	template <typename T>
	void SerializeTypeBinary(std::ostream& stream, const T& data)
	{
		SerializeTypeBinary(stream, &data, TypeId::Create<std::remove_cvref_t<T>>());
	}

	/** DESERIALIZER */

	inline void DeserializeType(std::istream& stream, void* data, TypeId type)
	{
		auto& info = GetTypeInfo(id);

		if (info.Deserializer)
		{
			info.Deserializer(stream, data);
		}
		else
		{
			std::string MemberName{};
			MemberName.reserve(64);

			char buffer{};
			stream >> buffer;
			assert(buffer == '{');

			auto& members = info.Members;
			while (buffer != '}')
			{
				Deserialize(stream, MemberName);

				auto member = std::find_if(members.begin(), members.end(), [&MemberName](const MemberInfo& info) {return info.Name == MemberName; });
				if (member != members.end() && !member->VariableId.IsRefOrPointer())
				{
					stream >> buffer;
					assert(buffer == ':');
					DeserializeType(stream, VoidOffset(data, member->Offset), member->VariableId.GetTypeId());
				}

				stream >> buffer;
			}
		}
	}

	template <typename T>
	void DeserializeType(std::istream& stream, T& data)
	{
		DeserializeType(stream, &data, TypeId::Create<std::remove_cvref_t<T>>());
	}

	inline void DeserializeTypeBinary(std::istream& stream, void* data, TypeId type)
	{
		auto& info = GetTypeInfo(id);

		if (info.BinaryDeserializer)
		{
			info.BinaryDeserializer(stream, data);
		}
		else
		{
			auto& members = info.Members;
			for (auto& member : members)
			{
				if (!member.VariableId.IsRefOrPointer())
				{
					DeserializeTypeBinary(stream, VoidOffset(data, member.Offset), member.VariableId.GetTypeId());
				}
			}
		}
	}

	template <typename T>
	void DeserializeTypeBinary(std::istream& stream, T& data)
	{
		DeserializeTypeBinary(stream, &data, TypeId::Create<std::remove_cvref_t<T>>());
	}

	/** STRING */

	inline void Serialize(std::ostream& stream, const std::string& value)
	{
		stream << '\"' << value << '\"';
	}

	inline void Deserialize(std::istream& stream, std::string& value)
	{
		value.clear();
		char buffer{};

		stream >> buffer;
		assert(buffer == '"');

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

	/** CONTAINER HELPERS */

	template <typename Container>
	inline void SerializeContainer(std::ostream& stream, const Container& container)
	{
		stream << '[';

		int counter{};
		for (auto& val : container)
		{
			if (counter++ != 0)
				stream << ',';

			SerializeType(stream, val);
		}

		stream << ']';
	}

	template <typename Container>
	inline void SerializeMapContainer(std::ostream& stream, const Container& container)
	{
		stream << '{';

		int counter{};
		for (auto& [key, val] : container)
		{
			if (counter++ != 0)
				stream << ',';

			stream << '\"';
			SerializeType(stream, key);
			stream << "\": ";
			SerializeType(stream, val);
		}

		stream << '}';
	}

	template <typename Container, typename ContainerType>
	inline void DeserializeContainer(std::istream& stream, Container& container, const std::function<void(Container&, ContainerType&&)>& addingFunction)
	{
		char buffer{};
		stream >> buffer;
		assert(buffer == '[');

		while (buffer != ']')
		{
			typename Container::value_type entry{};
			DeserializeType(stream, entry);
			addingFunction(container, std::move(entry));

			stream >> buffer;
		}
	}

	template <typename Container>
	inline void DeserializeMapContainer(std::istream& stream, Container& container)
	{
		char buffer{};
		stream >> buffer;
		assert(buffer == '{');

		while (buffer != '}')
		{
			std::string keyVal{};
			DeserializeType(stream, keyVal);
			std::stringstream keyStream{ std::move(keyVal) };

			std::remove_const_t<typename Container::key_type> key{};
			keyStream >> key;

			stream >> buffer;
			assert(buffer == ':');

			DeserializeType(stream, container.emplace(std::move(key), std::move(typename Container::value_type::second_type{})).first->second);

			stream >> buffer;
		}
	}

	template <typename Container>
	inline void SerializeBinaryContainer(std::ostream& stream, const Container& container)
	{
		size_t size = container.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

		for (auto& entry : container)
		{
			SerializeTypeBinary(stream, entry);
		}
	}

	template <typename Container>
	inline void SerializeBinaryMapContainer(std::ostream& stream, const Container& container)
	{
		size_t size = container.size();
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

		for (auto& [key, value] : container)
		{
			SerializeTypeBinary(stream, key);
			SerializeTypeBinary(stream, value);
		}
	}

	template <typename Container, typename ContainerType>
	inline void DeserializeBinaryContainer(std::istream& stream, Container& container, const std::function<void(Container&, ContainerType&&)>& addingFunction)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		for (size_t i{}; i < size; ++i)
		{
			typename Container::value_type entry{};
			DeserializeTypeBinary(stream, entry);
			addingFunction(container, std::move(entry));
		}
	}

	template <typename Container>
	inline void DeserializeBinaryMapContainer(std::istream& stream, Container& container)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		for (size_t i{}; i < size; ++i)
		{
			std::remove_const_t<typename Container::value_type::first_type> key{};
			typename Container::value_type::second_type value{};
			DeserializeTypeBinary(stream, key);
			DeserializeTypeBinary(stream, value);
			container.emplace(std::move(key), std::move(value));
		}
	}

	/** VECTOR */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::vector<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::vector<T>& value)
	{
		std::function<void(std::vector<T>&, T&&)> addingFunction = [](std::vector<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::vector<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::vector<T>& value)
	{
		size_t size{};
		stream.read(reinterpret_cast<char*>(&size), sizeof(size_t));

		value.reserve(size);

		for (size_t i{}; i < size; ++i)
		{
			T entry{};
			DeserializeTypeBinary(stream, entry);
			value.emplace_back(std::move(entry));
		}
	}

	/** ARRAY */

	template <typename T, size_t size>
	inline void Serialize(std::ostream& stream, const std::array<T, size>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T, size_t size>
	inline void Deserialize(std::istream& stream, std::array<T, size>& value)
	{
		char buffer{};
		stream >> buffer;
		assert(buffer == '[');

		size_t counter{};
		while (buffer != ']')
		{
			DeserializeType(stream, value[counter]);

			stream >> buffer;
			++counter;
		}
	}
	template <typename T, size_t size>
	inline void SerializeBinary(std::ostream& stream, const std::array<T, size>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T, size_t size>
	inline void DeserializeBinary(std::istream& stream, std::array<T, size>& value)
	{
		size_t arraySize{};
		stream.read(reinterpret_cast<char*>(&arraySize), sizeof(size_t));
		assert(arraySize == size);

		for (size_t i{}; i < size; ++i)
		{
			DeserializeTypeBinary(stream, value[i]);
		}
	}

	/** DEQUE */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::deque<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::deque<T>& value)
	{
		std::function<void(std::deque<T>&, T&&)> addingFunction = [](std::deque<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::deque<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::deque<T>& value)
	{
		std::function<void(std::deque<T>&, T&&)> addingFunction = [](std::deque<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** FORWARD LIST */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::forward_list<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::forward_list<T>& value)
	{
		std::function<void(std::forward_list<T>&, T&&)> addingFunction = [](std::forward_list<T>& container, T&& val) { container.emplace_front(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::forward_list<T>& value)
	{
		size_t size = std::distance(value.begin(), value.end());
		stream.write(reinterpret_cast<const char*>(&size), sizeof(size_t));

		for (auto& entry : value)
		{
			SerializeTypeBinary(stream, entry);
		}
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::forward_list<T>& value)
	{
		std::function<void(std::forward_list<T>&, T&&)> addingFunction = [](std::forward_list<T>& container, T&& val) { container.emplace_front(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** LIST */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::list<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::list<T>& value)
	{
		std::function<void(std::list<T>&, T&&)> addingFunction = [](std::list<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::list<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::list<T>& value)
	{
		std::function<void(std::list<T>&, T&&)> addingFunction = [](std::list<T>& container, T&& val) { container.emplace_back(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** SET */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::set<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::set<T>& value)
	{
		std::function<void(std::set<T>&, T&&)> addingFunction = [](std::set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::set<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::set<T>& value)
	{
		std::function<void(std::set<T>&, T&&)> addingFunction = [](std::set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** MULTISET */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::multiset<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::multiset<T>& value)
	{
		std::function<void(std::multiset<T>&, T&&)> addingFunction = [](std::multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::multiset<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::multiset<T>& value)
	{
		std::function<void(std::multiset<T>&, T&&)> addingFunction = [](std::multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** UNORDERED SET */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::unordered_set<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::unordered_set<T>& value)
	{
		std::function<void(std::unordered_set<T>&, T&&)> addingFunction = [](std::unordered_set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_set<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::unordered_set<T>& value)
	{
		std::function<void(std::unordered_set<T>&, T&&)> addingFunction = [](std::unordered_set<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** UNORDERED MULTISET */

	template <typename T>
	inline void Serialize(std::ostream& stream, const std::unordered_multiset<T>& value)
	{
		SerializeContainer(stream, value);
	}
	template <typename T>
	inline void Deserialize(std::istream& stream, std::unordered_multiset<T>& value)
	{
		std::function<void(std::unordered_multiset<T>&, T&&)> addingFunction = [](std::unordered_multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeContainer(stream, value, addingFunction);
	}
	template <typename T>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_multiset<T>& value)
	{
		SerializeBinaryContainer(stream, value);
	}
	template <typename T>
	inline void DeserializeBinary(std::istream& stream, std::unordered_multiset<T>& value)
	{
		std::function<void(std::unordered_multiset<T>&, T&&)> addingFunction = [](std::unordered_multiset<T>& container, T&& val) { container.emplace(std::move(val)); };
		DeserializeBinaryContainer(stream, value, addingFunction);
	}

	/** MAP */

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::map<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::map<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::map<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::map<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}

	/** MULTIMAP */

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::multimap<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::multimap<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::multimap<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::multimap<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}

	/** UNORDERED MAP */

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::unordered_map<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::unordered_map<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_map<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::unordered_map<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}

	/** UNORDERED MULTIMAP */

	template <typename Key, typename Value>
	inline void Serialize(std::ostream& stream, const std::unordered_multimap<Key, Value>& value)
	{
		SerializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void Deserialize(std::istream& stream, std::unordered_multimap<Key, Value>& value)
	{
		DeserializeMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void SerializeBinary(std::ostream& stream, const std::unordered_multimap<Key, Value>& value)
	{
		SerializeBinaryMapContainer(stream, value);
	}
	template <typename Key, typename Value>
	inline void DeserializeBinary(std::istream& stream, std::unordered_multimap<Key, Value>& value)
	{
		DeserializeBinaryMapContainer(stream, value);
	}

}

