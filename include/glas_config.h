#pragma once

#include <functional>
#include <bitset>
#include <set>

#include "glas_decl.h"

/**
 * Comment to disable Addon and unconmment to enable
 */

#define GLAS_STORAGE
#define GLAS_SERIALIZATION

/**
 * struct holds type information about each type
 */

namespace glas
{
	struct TypeInfo final
	{
		std::string_view				Name	{ };
		uint32_t						Size	{ };
		uint32_t						Align	{ };

		std::set<MemberInfo>			Members	{ };

#ifdef GLAS_STORAGE
		void (*Constructor)			(void*)							{ };
		void (*MoveConstructor)		(void*, void*)					{ };
		void (*CopyConstructor)		(void*, void*)					{ };
		void (*Destructor)			(void*)							{ };
#endif // GLASS_STORAGE
#ifdef GLAS_SERIALIZATION
		void (*Serializer)			(std::ostream&, const void*)	{ };
		void (*BinarySerializer)	(std::ostream&, const void*)	{ };
		void (*Deserializer)		(std::istream&, void*)			{ };
		void (*BinaryDeserializer)	(std::istream&, void*)			{ };
#endif // GLAS_SERIALIZATION

		/**
		 * Add custom Type Information variables here
		 */



		/**
		 * End of Custom Type Information Variables
		 */

		template <typename T>
		static constexpr TypeInfo Create();
	};
}


#ifdef GLAS_STORAGE
#include "storage/glas_storage_config.h"
#endif // GLASS_STORAGE
#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization_config.h"
#endif // GLAS_SERIALIZATION

/**
 * Type Info Variables Initialization
 */

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

#ifdef GLAS_STORAGE
		Storage::FillInfo<T>(info);
#endif // GLAS_STORAGE
#ifdef GLAS_SERIALIZATION
		Serialization::FillInfo<T>(info);
#endif // GLAS_SERIALIZATION

		/**
		 * Begin initialization of Custom Information variables here
		 */



		/**
		 * End  initialization of Custom Type Information Variables
		 */

		return info;
	}
}



#ifdef GLAS_SERIALIZATION
#include "serialization/glas_serialization.h"
#endif // GLAS_SERIALIZATION
#ifdef GLAS_STORAGE
#include "storage/glas_storage.h"
#endif // GLAS_SERIALIZATION