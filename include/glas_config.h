#pragma once

#include <functional>
#include <bitset>
#include <set>

#include "glas_decl.h"

#define GLAS_SERIALIZATION



namespace glas
{
	struct TypeInfo final
	{
		std::string_view				Name	{ };
		uint32_t						Size	{ };
		uint32_t						Align	{ };

		std::set<MemberInfo>			Members	{ };

#ifdef GLAS_SERIALIZATION
		void (*Serializer)			(std::ostream&, const void*)	{ };
		void (*BinarySerializer)	(std::ostream&, const void*)	{ };
		void (*Deserializer)		(std::istream&, void*)			{ };
		void (*BinaryDeserializer)	(std::istream&, void*)			{ };
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

#ifdef GLAS_SERIALIZATION
		Serialization::FillInfo<T>(info);
#endif // GLAS_SERIALIZATION

		return info;
	}
}



#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization.h"
#endif // GLAS_SERIALIZATION