#pragma once

#include <iostream>
#include <cassert>
#include <stack>
#include <span>
#include <vector>

#define GLAS_MEM_TRACE_STRING
#define GLAS_MEM_TRACE_VECTOR
#define GLAS_MEM_TRACE_ARRAY
#define GLAS_MEM_TRACE_DEQUE
#define GLAS_MEM_TRACE_FORWARD_LIST
#define GLAS_MEM_TRACE_LIST
#define GLAS_MEM_TRACE_SET
#define GLAS_MEM_TRACE_UNORDERED_SET
#define GLAS_MEM_TRACE_MAP
#define GLAS_MEM_TRACE_MEMORY
#define GLAS_MEM_TRACE_OPTIONAL
#define GLAS_MEM_TRACE_UTILITY

namespace glas
{
	class TypeId;
	struct TypeInfo;
}

namespace glas::MemTrace
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info);
}

namespace glas::MemTrace
{
	class MemoryStack final
	{
		struct MemoryEntry
		{
			TypeId		OwnerType		{ };
			const void*	Address			{ };
			int32_t		Size			{ };
			int32_t		Id				{ };
			int32_t		Parent			{ };
			int32_t		UserDataOffset	{ -1 };

			constexpr bool IsTop() const { return Parent == -1; }

			bool operator<(const MemoryEntry& other) const
			{
				return this->Address < other.Address;
			}
		};


	public:

		template <typename T>
		void Initialize(const T& value);
		void Initialize(const void* address, size_t size);

		template <typename OwningType, typename UserData = void>
		void PushMemoryArea(const void* address, size_t size, const UserData& data);
		void PushMemoryArea(TypeId ownerType, const void* address, size_t size, std::span<const uint8_t> userData = {});
		void PopMemoryArea();

		[[nodiscard]] const MemoryEntry* GetMemoryEntry(const void* address) const;
		const MemoryEntry& GetParent(const MemoryEntry& entry) const;

		[[nodiscard]] size_t AddUserData(const void* data, size_t size);

		const void* GetUserData(size_t offset) const;

		template <typename T>
		const void* GetMappedAddress(const void* oldAddress, const T& parent);
		const void* GetMappedAddress(const void* oldAddress, const void* parent);

	private:
		void Sort();

	private:
		std::stack<MemoryEntry>			m_Stack		{ };
		std::vector<MemoryEntry>		m_Nodes		{ };
		std::vector<uint8_t>			m_UserData	{ };
	};
}

/** STRING */
#if defined(GLAS_MEM_TRACE_STRING) || defined(_STRING_)
#include <string>
namespace glas::MemTrace
{
	inline void TraceMemory(MemoryStack& tracer, const std::string& value);
	inline const void* GetAddress(const void* userData, const std::string& value);
}
#endif

/** VECTOR */
#if defined(GLAS_MEM_TRACE_VECTOR) || defined(_VECTOR_)
#include <vector>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::vector<T>& value);
	template <typename T>
	const void* GetAddress(const void* userData, const std::vector<T>& value);
}
#endif

/** ARRAY */
#if defined(GLAS_MEM_TRACE_ARRAY) || defined(_ARRAY_)
#include <array>
namespace glas::MemTrace
{
	template <typename T, size_t size>
	void TraceMemory(MemoryStack& tracer, const std::array<T, size>& value);
}
#endif

/** DEQUE */
#if defined(GLAS_MEM_TRACE_DEQUE) || defined(_DEQUE_)
#include <deque>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::deque<T>& value);
}
#endif

/** FORWARD LIST */
#if defined(GLAS_MEM_TRACE_FORWARD_LIST) || defined(_FORWARD_LIST_)
#include <forward_list>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::forward_list<T>& value);
}
#endif

/** LIST */
#if defined(GLAS_MEM_TRACE_LIST) || defined(_LIST_)
#include <list>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::list<T>& value);
	template <typename T>
	const void* GetAddress(const void* userData, const std::list<T>& value);
}
#endif

/** SET & MULTI SET */
#if defined(GLAS_MEM_TRACE_SET) || defined(_SET_)
#include <set>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::set<T>& value);

	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::multiset<T>& value);
}
#endif

/** UNORDERED SET */
#if defined(GLAS_MEM_TRACE_UNORDERED_SET) || defined(_UNORDERED_SET_)
#include <unordered_set>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::unordered_set<T>& value);

	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::unordered_multiset<T>& value);
}
#endif

/** MAP */
#if defined(GLAS_MEM_TRACE_MAP) || defined(_MAP_)
#include <map>
namespace glas::MemTrace
{
	template <typename Key, typename Value>
	void TraceMemory(MemoryStack& tracer, const std::map<Key, Value>& value);

	template <typename Key, typename Value>
	void TraceMemory(MemoryStack& tracer, const std::multimap<Key, Value>& value);
}
#endif

/** UNORDERED MAP */
#if defined(GLAS_MEM_TRACE_MAP) || defined(_UNORDERED_MAP_)
#include <unordered_map>
namespace glas::MemTrace
{
	template <typename Key, typename Value>
	void TraceMemory(MemoryStack& tracer, const std::unordered_map<Key, Value>& value);

	template <typename Key, typename Value>
	void TraceMemory(MemoryStack& tracer, const std::unordered_multimap<Key, Value>& value);
}
#endif

/** MEMORY */
#if defined(GLAS_MEM_TRACE_MEMORY) || defined(_MEMORY_)
#include <memory>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::unique_ptr<T>& value);
}
#endif

/** OPTIONAL */
#if defined(GLAS_MEM_TRACE_OPTIONAL) || defined(_OPTIONAL_)
#include <optional>
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::optional<T>& value);
}
#endif

/** UTILITY */
#if defined(GLAS_MEM_TRACE_UTILITY) || defined(_UTILITY_)
#include <utility>
namespace glas::MemTrace
{
	template <typename T1, typename T2>
	void TraceMemory(MemoryStack& tracer, const std::pair<T1, T2>& value);

	template <typename... Ts>
	void TraceMemory(MemoryStack& tracer, const std::tuple<Ts...>& value);
}
#endif

namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const T& value) requires std::is_fundamental_v<T>
	{}

	template <typename T>
	void TraceMemoryBinary(MemoryStack& tracer, const T& value) requires std::is_trivially_copyable_v<T>
	{}
}

/** STORAGE */
#ifdef GLAS_STORAGE
#include "../storage/glas_storage_config.h"
namespace glas::MemTrace
{
	inline void TraceMemory(MemoryStack& tracer, const Storage::TypeStorage& value);

	inline void TraceMemory(MemoryStack& tracer, const Storage::TypeTuple& value);
}
#endif


/** DEFAULT MEM_TRACE */
namespace glas::MemTrace
{
	void TraceMemory(MemoryStack& tracer, const void* data, TypeId type);

	template <typename T>
	void TraceMemory(MemoryStack& tracer, const T& value);
}