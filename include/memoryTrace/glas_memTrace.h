#pragma once

#include "glas_memTrace_config.h"
#include "../glas_config.h"

namespace glas::MemTrace
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info)
	{
		
	}

	// HELPERS

	constexpr size_t VoidDistance(const void* final, const void* initial)
	{
		return static_cast<const uint8_t*>(final) - static_cast<const uint8_t*>(initial);
	}
}

namespace glas::MemTrace
{
	template <typename T>
	void MemoryStack::Initialize(const T& value)
	{
		Initialize(&value, sizeof(T));
	}

	inline void MemoryStack::Initialize(const void* address, size_t size)
	{
		// make sure stack is empty
		assert(m_Stack.empty());

		// add first element into stack, with ID:0
		// set -1 as the parent of this id
		const MemoryEntry& entry = m_Stack.emplace(0, address, size, 0, -1, 0);

		m_Nodes.emplace_back(entry);
	}

	inline void MemoryStack::PushMemoryArea(TypeId ownerType, const void* address, size_t size, std::span<const uint8_t> userData)
	{
		// make sure the stack is initialized
		assert(!m_Stack.empty());

		MemoryEntry entry{};
		entry.OwnerType = ownerType;
		entry.Address	= address;
		entry.Size		= size;
		entry.Id		= m_Nodes.size() - 1;
		entry.Parent	= m_Stack.top().Id;
		
		if (!userData.empty())
		{
			entry.UserDataOffset = static_cast<int32_t>(AddUserData(userData));
		}

		// add the node to the stack with its ID the same value as the current size of m_Parents - 1
		m_Stack.emplace(entry);
		m_Nodes.emplace_back(entry);
	}

	inline void MemoryStack::PopMemoryArea()
	{
		assert(!m_Stack.empty());
		m_Stack.pop();
	}

	inline const MemoryStack::MemoryEntry* MemoryStack::GetMemoryEntry(const void* address) const
	{
		for (auto& entry : m_Nodes)
		{
			if (address >= entry.Address && VoidDistance(address, entry.Address) < entry.Size)
			{
				return &entry;
			}
		}
		return nullptr;
	}

	inline const MemoryStack::MemoryEntry& MemoryStack::GetParent(const MemoryEntry& entry) const
	{
		assert(entry.Parent != -1 && entry.Parent < m_Nodes.size());
		return m_Nodes[entry.Parent];
	}

	inline size_t MemoryStack::AddUserData(const void* data, size_t size)
	{
		size_t dataLocationOffset = m_UserData.size();
		m_UserData.insert(m_UserData.end(), size, 0);

		std::memcpy(m_UserData.data() + dataLocationOffset, data, size);

		return dataLocationOffset;
	}

	inline const void* MemoryStack::GetUserData(size_t offset) const
	{
		return &m_UserData[offset];
	}

	template <typename T>
	const void* MemoryStack::GetMappedAddress(const void* oldAddress, const T& parent)
	{
		return GetMappedAddress(oldAddress, &parent, TypeId::Create<T>());
	}

	inline const void* MemoryStack::GetMappedAddress(const void* oldAddress, const void* baseObject)
	{
		if (auto entry = GetMemoryEntry(oldAddress))
		{
			std::vector<size_t> parentTree;

			do
			{
				parentTree.emplace_back(entry->Id);

				entry = &GetParent(*entry);

			} while (!entry->IsTop());

			for (auto it{ m_Nodes.rbegin() }; it != m_Nodes.rend(); ++it)
			{
				auto& info = it->OwnerType.GetInfo();
				info.
			}
		}

		return nullptr;
	}

	inline void MemoryStack::Sort()
	{
		
	}
}

/** STRING */
#if defined(GLAS_MEM_TRACE_STRING) || defined(_STRING_)
namespace glas::MemTrace
{
	inline void TraceMemory(MemoryStack& tracer, const std::string& value)
	{
		tracer.PushMemoryArea(TypeId::Create<std::string>(), value.data(), value.size() * sizeof(char));

		tracer.PopMemoryArea();
	}

	inline const void* GetAddress(const void*, const std::string& value)
	{
		return value.data();
	}
}
#endif

/** VECTOR */
#if defined(GLAS_MEM_TRACE_VECTOR) || defined(_VECTOR_)
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::vector<T>& value)
	{
		tracer.PushMemoryArea(TypeId::Create<std::vector<T>>(), & value, value.data(), value.size() * sizeof(T));

		for (auto& element : value)
		{
			TraceMemory(&element, tracer, element);
		}

		tracer.PopMemoryArea();
	}

	template <typename T>
	const void* GetAddress(const void* userData, const std::vector<T>& value)
	{
		return value.data();
	}
}
#endif

/** LIST */
#if defined(GLAS_MEM_TRACE_LIST) || defined(_LIST_)
namespace glas::MemTrace
{
	template <typename T>
	void TraceMemory(MemoryStack& tracer, const std::list<T>& value)
	{
		auto it = value.begin();

		for (size_t i{}; i < value.size(); ++i)
		{
			tracer.PushMemoryArea(TypeId::Create<std::list<T>>(), &*it, )
		}
	}

	template <typename T>
	const void* GetAddress(const void* userData, const std::list<T>& value)
	{
		for (auto& element : value)
		{
			
		}
	}
}
#endif