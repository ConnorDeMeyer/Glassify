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
		const MemoryEntry& entry = m_Stack.emplace(0, address, size, 0, -1);

		m_Nodes.emplace_back(entry);
	}

	inline void MemoryStack::PushMemoryArea(const void* owner, const void* address, size_t size)
	{
		// make sure the stack is initialized
		assert(!m_Stack.empty());

		// add the node to the stack with its ID the same value as the current size of m_Parents - 1
		const MemoryEntry& entry = m_Stack.emplace(VoidDistance(m_Stack.top().Address, owner), address, size, ++m_IdCounter, m_Stack.top().Id);
		m_Nodes.emplace_back(entry);

		assert(m_IdCounter == m_Nodes.size() - 1);
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
		tracer.PushMemoryArea(&value, value.data(), value.size() * sizeof(char));

		tracer.PopMemoryArea();
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
		tracer.PushMemoryArea(&value, value.data(), value.size() * sizeof(T));

		for (auto& element : value)
		{
			TraceMemory(&element, tracer, element);
		}

		tracer.PopMemoryArea();
	}
}
#endif