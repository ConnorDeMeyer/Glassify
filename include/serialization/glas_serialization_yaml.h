#pragma once

#ifdef GLAS_SERIALIZATION_YAML

#include "../glas_decl.h"
#include "glas_serialization_config_yaml.h"

#include <functional>
#include <type_traits>

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoYaml(TypeInfo& info)
	{
		info.YamlSerializer = [](const void* data)
			{
				return YAML::Node{ *static_cast<const T*>(data) };
			};

		info.YamlDeserializer = [](const YAML::Node& node, void* data)
			{
				YAML::convert<T>::decode(node, *static_cast<T*>(data));
			};
	}

	inline void SerializeYaml(std::ostream& stream, const void* value, TypeId type)
	{
		YAML::Emitter emitter{ stream };

		auto& info = type.GetInfo();
		assert(info.YamlSerializer);
		YAML::Node node = info.YamlSerializer(value);

		emitter << node;
	}

	inline void DeserializeYaml(std::istream& stream, void* value, TypeId type)
	{
		YAML::Node node = YAML::Load(stream);

		//YAML::Emitter emitter{ std::cout };
		//emitter << node;

		auto& info = type.GetInfo();
		assert(info.YamlDeserializer);
		info.YamlDeserializer(node, value);
	}

	template <typename T>
	void SerializeYaml(std::ostream& stream, const T& value)
	{
		SerializeYaml(stream, &value, TypeId::Create<T>());
	}

	template <typename T>
	void DeserializeYaml(std::istream& stream, T& value)
	{
		DeserializeYaml(stream, &value, TypeId::Create<T>());
	}

	YAML::Node SerializeYamlDefault(const void* data, glas::TypeId type)
	{
		YAML::Node node; 

		auto& info = type.GetInfo();
		for (auto& member : info.Members)
		{
			if (!member.Variable.IsRefOrPointer() && member.Variable.GetTypeId().IsValid())
			{
				auto& memberInfo = member.Variable.GetTypeId().GetInfo();

				node[member.Name] = memberInfo.YamlSerializer(VoidOffset(data, member.Offset));
			}
		}

		return node;
	}

	bool DeserializeYamlDefault(const YAML::Node& node, void* data, glas::TypeId type)
	{
		auto& info = type.GetInfo();

		for (auto& member : info.Members)
		{
			if (!member.Variable.IsRefOrPointer() && member.Variable.GetTypeId().IsValid())
			{
				auto& memberInfo = member.Variable.GetTypeId().GetInfo();

				if (auto varNode = node[member.Name])
				{
					memberInfo.YamlDeserializer(varNode, VoidOffset(data, member.Offset));
				}
			}
		}

		return true;
	}

}

namespace YAML
{
	template <typename T>
	Node convert<T>::encode(const T& value)
	{
		return glas::Serialization::SerializeYamlDefault(&value, glas::TypeId::Create<T>());
	}

	template <typename T>
	bool convert<T>::decode(const Node& node, T& value)
	{
		return glas::Serialization::DeserializeYamlDefault(node, &value, glas::TypeId::Create<T>());
	}
}

#endif