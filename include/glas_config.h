#pragma once

#include <functional>
#include <bitset>
#include <set>

#include "glas_decl.h"

#define GLAS_SERIALIZATION
#ifdef GLAS_SERIALIZATION
#define GLAS_SERIALIZE_FILL(info) Serialization::FillInfo<T>(info);
#else 
#define GLAS_SERIALIZE_FILL(info) (void)info;
#endif // GLAS_SERIALIZATION



namespace glas
{
	struct TypeInfo final
	{
		std::string_view				Name{ };
		uint32_t						Size{ };
		uint32_t						Align{ };

		std::set<MemberInfo>			Members{ };

#ifdef GLAS_SERIALIZATION
		std::function<void(std::ostream&, const void*)> Serializer{};
		std::function<void(std::ostream&, const void*)> BinarySerializer{};
		std::function<void(std::istream&, void*)> Deserializer{};
		std::function<void(std::istream&, void*)> BinaryDeserializer{};
#endif // GLAS_SERIALIZATION


		template <typename T>
		static constexpr TypeInfo Create();
	};
}


#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization_config.h"
#endif // GLAS_SERIALIZATION


namespace glas
{
	template<typename T>
	inline constexpr TypeInfo TypeInfo::Create()
	{
		TypeInfo info{};

		info.Name = TypeName<T>();

		if constexpr (!std::is_same_v<void, T>)
		{
			info.Size = sizeof(T);
			info.Align = alignof(T);
		}

		GLAS_SERIALIZE_FILL(info);

		return info;
	}
}



#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization.h"
#endif // GLAS_SERIALIZATION