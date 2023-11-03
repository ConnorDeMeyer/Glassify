#pragma once

#ifdef GLAS_SERIALIZATION_YAML

#include <cassert>
#include <sstream>
#include <type_traits>

#include "../glas_decl.h"

#ifdef GLAS_STORAGE
#include "../storage/glas_storage_config.h"
#endif

//#include "YAML3rdParty/yaml.h"
#include "YAML3rdParty/yaml.h"

namespace glas
{
	struct TypeInfo;
}

namespace glas::Serialization
{
	template <typename T>
	constexpr void FillTypeInfoYaml(TypeInfo& info);
}

namespace glas::Serialization
{
	template <typename T>
	struct YAMLSerializer;
}

namespace glas::Serialization
{
	inline void SerializeYaml(std::ostream& stream, const void* value, TypeId type);
	inline void DeserializeYaml(std::istream& stream, void* value, TypeId type);

	template <typename T>
	void SerializeYaml(std::ostream& stream, const T& value);
	template <typename T>
	void DeserializeYaml(std::istream& stream, T& value);

	inline YAML::Node SerializeYamlDefault(const void* data, glas::TypeId type);
	inline bool DeserializeYamlDefault(const YAML::Node& node, void* data, glas::TypeId type);
}

namespace YAML
{
    /**
     * DEFAULT
     */
    template<typename T>
    struct convert
	{
        static Node encode(const T& value);

        static bool decode(const Node& node, T& rhs);
    };

}

#endif