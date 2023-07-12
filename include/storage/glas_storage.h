#pragma once

#include "glas_storage_config.h"
#include "../glas_config.h"
#include <stdexcept>

namespace glas::Storage
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info)
	{
		if constexpr (std::is_default_constructible_v<T>)
			info.Constructor = [](void* location)
		{
			new (location) T();
		};

		if constexpr (EnableCopyConstructor<T>)
			info.CopyConstructor = [](void* location, const void* other)
		{
			new (location) T(*static_cast<const T*>(other));
		};

		if constexpr (EnableMoveConstructor<T>)
			info.MoveConstructor = [](void* location, void* other)
		{
			new (location) T(std::move(*static_cast<T*>(other)));
		};

		if constexpr (std::is_destructible_v<T>)
			info.Destructor = [](void* data)
		{
			static_cast<T*>(data)->~T();
		};

		if constexpr (Swappable<T>)
			info.Swap = [](void* lhs, void* rhs)
		{
			std::swap(*static_cast<T*>(lhs), *static_cast<T*>(rhs));
		};
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

	/**
	 * TYPE STORAGE
	 */

	inline TypeStorage::TypeStorage(glas::TypeId id)
		: m_TypeId{ id }
	{
		auto& info = id.GetInfo();
		m_Data = std::make_unique<uint8_t[]>(info.Size);
		assert(info.Constructor && "Type has no default constructor");
		info.Constructor(m_Data.get());
	}

	inline TypeStorage::~TypeStorage()
	{
		if (m_TypeId.IsValid() && m_Data)
		{
			auto& info = m_TypeId.GetInfo();
			if (info.Destructor)
			{
				info.Destructor(m_Data.get());
			}
		}
	}

	inline TypeStorage::TypeStorage(TypeStorage&& other) noexcept
		: m_Data{ std::move(other.m_Data) }
		, m_TypeId{ other.m_TypeId }
	{
		other.m_TypeId = {};
	}

	inline TypeStorage& TypeStorage::operator=(TypeStorage&& other) noexcept
	{
		if (this != &other)
		{
			m_Data = std::move(other.m_Data);
			m_TypeId = other.m_TypeId;

			other.m_TypeId = {};
		}
		return *this;
	}

	template <typename T>
	TypeStorage TypeStorage::Construct() requires std::is_default_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.m_Data.get()) T();

		return storage;
	}

	template <typename T>
	TypeStorage TypeStorage::CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.m_Data.get()) T(value);

		return storage;
	}

	template <typename T>
	TypeStorage TypeStorage::MoveConstruct(T&& value) requires std::is_move_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.m_Data.get()) T(std::move(value));

		return storage;
	}

	inline TypeStorage TypeStorage::CopyConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.CopyConstructor);

		TypeStorage storage;

		storage.m_TypeId = id;
		storage.m_Data = std::make_unique<uint8_t[]>(info.Size);

		info.CopyConstructor(storage.GetData(), original);
		return storage;
	}

	inline TypeStorage TypeStorage::MoveConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.MoveConstructor);

		TypeStorage storage;

		storage.m_TypeId = id;
		storage.m_Data = std::make_unique<uint8_t[]>(info.Size);

		info.MoveConstructor(storage.GetData(), original);
		return storage;
	}

	template <typename T>
	T* TypeStorage::As() const
	{
		return (TypeId::Create<T>() == m_TypeId) ? static_cast<T*>(GetData()) : nullptr;
	}

	/**
	 * SHARED TYPE STORAGE
	 */

	inline SharedTypeStorage::SharedTypeStorage(glas::TypeId id)
		: m_TypeId{ id }
	{
		auto& info = id.GetInfo();
		m_Data = std::make_shared<uint8_t[]>(info.Size);
		assert(info.Constructor && "Type has no default constructor");
		info.Constructor(m_Data.get());
	}

	inline SharedTypeStorage::~SharedTypeStorage()
	{
		if (m_TypeId.IsValid() && m_Data.use_count() == 1)
		{
			auto& info = m_TypeId.GetInfo();
			if (info.Destructor)
			{
				info.Destructor(m_Data.get());
			}
		}
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::Construct() requires std::is_default_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_shared<uint8_t[]>(sizeof(T));

		new (storage.m_Data.get()) T();

		return storage;
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.m_Data.get()) T(value);

		return storage;
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::MoveConstruct(T&& value) requires std::is_move_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.m_Data.get()) T(std::move(value));

		return storage;
	}

	inline SharedTypeStorage SharedTypeStorage::CopyConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.CopyConstructor);

		SharedTypeStorage storage;

		storage.m_TypeId = id;
		storage.m_Data = std::make_unique<uint8_t[]>(info.Size);

		info.CopyConstructor(storage.GetData(), original);
		return storage;
	}

	inline SharedTypeStorage SharedTypeStorage::MoveConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.MoveConstructor);

		SharedTypeStorage storage;

		storage.m_TypeId = id;
		storage.m_Data = std::make_unique<uint8_t[]>(info.Size);

		info.MoveConstructor(storage.GetData(), original);
		return storage;
	}

	inline WeakTypeStorage::WeakTypeStorage(const SharedTypeStorage& sharedStorage)
		: m_Data{ sharedStorage.m_Data }
		, m_TypeId{ sharedStorage.m_TypeId }
	{

	}

	template <typename T>
	T* SharedTypeStorage::As()
	{
		return (TypeId::Create<T>() == m_TypeId) ? static_cast<T*>(GetData()) : nullptr;
	}

	/**
	 * WEAK TYPE STORAGE
	 */

	inline SharedTypeStorage WeakTypeStorage::GetSharedStorage() const
	{
		SharedTypeStorage returnVal = { m_TypeId };
		if (!Expired())
		{
			returnVal.m_Data = m_Data.lock();
		}

		return returnVal;
	}

	/**
	 * TYPE TUPLE
	 */

	/**
	 * Structure of TypeTuple:
	 *
	 * Variable data jump table:
	 *		First section is a jump table that maps to the variable data
	 * 
	 * VariableIds information:
	 *		Second section stores all the VariableIds
	 *
	 * m_Data Section
	 *		Third section stores the variable data
	 */

	constexpr uint32_t GetOffset(uint32_t structAlign, uint32_t typeAlign, uint32_t currentOffset)
	{
		return ((currentOffset % structAlign) + typeAlign > structAlign) ?
			(currentOffset / structAlign + 1) * structAlign :
			currentOffset;
	}

	static_assert(GetOffset(8, 2, 8) == 8);
	static_assert(GetOffset(8, 2, 10) == 10);
	static_assert(GetOffset(8, 4, 10) == 10);
	static_assert(GetOffset(8, 8, 8) == 8);
	static_assert(GetOffset(8, 8, 9) == 16);
	static_assert(GetOffset(8, 8, 17) == 24);

	template <size_t Index, typename... Types>
	void FillDataCopy(TypeTuple& typeTuple, const std::tuple<Types...>& tuple)
	{
		using Type = std::tuple_element_t<Index, std::tuple<Types...>>;
		typeTuple.Get<Type>(Index) = std::get<Index>(tuple);
		if constexpr (Index + 1 < sizeof...(Types))
		{
			FillDataCopy<Index + 1, Types...>(typeTuple, tuple);
		}
	}

	template <size_t Index, typename... Types>
	void FillDataMove(TypeTuple& typeTuple, std::tuple<Types...>&& tuple)
	{
		using Type = std::tuple_element_t<Index, std::tuple<Types...>>;
		typeTuple.Get<Type>(Index) = std::move(std::get<Index>(tuple));
		if constexpr (Index + 1 < sizeof...(Types))
		{
			FillDataMove<Index + 1, Types...>(typeTuple, std::move(tuple));
		}
	}

	inline TypeTuple::TypeTuple(std::span<VariableId> variables)
	{
		Initialize(variables, true);
	}

	template <typename ... T>
	TypeTuple::TypeTuple(const std::tuple<T...>& tuple)
	{
		auto variables = GetVariableArray<T...>();
		Initialize(variables, false);
		FillDataCopy<0, T...>(*this, tuple);
	}

	template <typename ... T>
	TypeTuple::TypeTuple(std::tuple<T...>&& tuple)
	{
		auto variables = GetVariableArray<T...>();
		Initialize(variables, false);
		FillDataMove<0, T...>(*this, std::move(tuple));
	}

	template <typename ... T>
	TypeTuple TypeTuple::Create()
	{
		//GetVariableIdArray<T...>();
		if constexpr (sizeof...(T) != 0)
		{
			auto variableArray = GetVariableArray<T...>();
			const auto variables = std::span<VariableId>(variableArray.begin(), variableArray.end());
			return TypeTuple(variables);
		}
		else
		{
			return {};
		}
	}

	inline VariableId TypeTuple::GetVariable(size_t index) const
	{
		return GetVariableIds()[index];
	}

	inline uint32_t TypeTuple::CalculateAlignment(std::span<VariableId> variables)
	{
		uint32_t maxAlign = 1;
		for (auto& variable : variables)
		{
			maxAlign = std::max(variable.GetAlign(), maxAlign);
		}
		return maxAlign;
	}

	inline void TypeTuple::Initialize(std::span<VariableId> variables, bool InitializeToDefault)
	{
		m_Size = static_cast<uint32_t>(variables.size());
		const size_t jumpTableSize{ sizeof(void*) * variables.size() };
		const size_t variableIdsSize{ sizeof(VariableId) * variables.size() };

		// Get the max alignment of all the classes
		const uint32_t structAlignment = CalculateAlignment(variables);

		// Calculate the final size of the datas
		uint32_t DataSize{};
		for (auto& var : variables)
		{
			DataSize = GetOffset(structAlignment, var.GetAlign(), DataSize) + var.GetSize();
		}

		// Allocate the memory
		m_Data = std::make_unique<uint8_t[]>(DataSize + jumpTableSize + variableIdsSize);

		// Copy the variable Ids to the allocated variable id section
		std::ranges::copy(variables, GetVariableIds().begin());

		// Populate the jump table
		uint32_t accumulatedOffset{};
		for (uint32_t i{}; i < variables.size(); ++i)
		{
			auto& var = variables[i];
			accumulatedOffset = GetOffset(structAlignment, var.GetAlign(), accumulatedOffset);

			reinterpret_cast<void**>(m_Data.get())[i] = VoidOffset(GetVariableDataPtr(), accumulatedOffset);

			accumulatedOffset += var.GetSize();
		}

		// Initialize the types
		if (InitializeToDefault)
		{
			for (uint32_t i{}; i < variables.size(); ++i)
			{
				auto& var = variables[i];

				if (!var.IsRefOrPointer())
				{
					if (const auto constructor = var.GetTypeId().GetInfo().Constructor)
					{
						constructor(GetVoid(i));
					}
				}
			}
		}
	}

	inline void* TypeTuple::GetVoid(size_t index) const
	{
		return reinterpret_cast<void**>(m_Data.get())[index];
	}

	template <typename T>
	T& TypeTuple::Get(size_t index)
	{
		assert(VariableId::Create<T>() == GetVariableIds()[index]);
		return *static_cast<T*>(reinterpret_cast<void**>(m_Data.get())[index]);
	}

	template <typename T>
	const T& TypeTuple::Get(size_t index) const
	{
		assert(VariableId::Create<T>() == GetVariableIds()[index]);
		return *static_cast<T*>(reinterpret_cast<void**>(m_Data.get())[index]);
	}

	/**
	 * TYPE VECTOR
	 * TYPE VECTOR
	 * TYPE VECTOR
	 */

	inline TypeVector::~TypeVector()
	{
		if (m_Data)
		{
			auto& destructor = m_ContainedType.GetInfo().Destructor;

			for (auto& element : *this)
			{
				destructor(element);
			}
		}
	}

	inline TypeVector::TypeVector(const TypeVector& other)
		: m_ContainedType	{ other.m_ContainedType }
		, m_ElementSize		{ other.m_ElementSize }
	{
		if (other.m_ContainedType.IsValid())
		{
			const auto& info = m_ContainedType.GetInfo();
			const auto& copyConstructor = info.CopyConstructor;
			assert(copyConstructor);
			m_ElementSize = info.Size;

			ResizeUninitialized(other.Size());

			for (size_t i{}; i < other.Size(); ++i)
			{
				copyConstructor((*this)[i], other[i]);
			}
		}
	}

	inline TypeVector::TypeVector(TypeVector&& other) noexcept
		: m_ContainedType	{ other.m_ContainedType }
		, m_Data			{ std::move(other.m_Data )}
		, m_Capacity		{ other.m_Capacity }
		, m_Size			{ other.m_Size }
		, m_ElementSize		{ other.m_ElementSize }
	{
		other.m_Capacity = 0;
		other.m_Size = 0;
	}

	inline TypeVector::TypeVector(TypeId type, size_t count)
		: m_ContainedType{ type }
	{
		assert(type.IsValid());
		assert(AssertType(type));

		const auto& info = type.GetInfo();
		const auto& constructor = info.Constructor;
		m_ElementSize = info.Size;

		ResizeUninitialized(count);
		for (auto& element : *this)
		{
			constructor(element);
		}
	}

	inline TypeVector::TypeVector(size_t count, TypeStorage& value)
		: TypeVector(value.GetType(), count, value.GetData())
	{}

	inline TypeVector::TypeVector(TypeId type, size_t count, const void* value)
		: m_ContainedType{ type }
	{
		assert(type.IsValid());
		assert(value);
		assert(AssertType(type));

		const auto& info = type.GetInfo();
		const auto& copyConstructor = info.CopyConstructor;
		m_ElementSize = info.Size;

		ResizeUninitialized(count);
		for (auto& element : *this)
		{
			copyConstructor(element, value);
		}
	}

	template <typename T>
	TypeVector::TypeVector(size_t count)
		: TypeVector(TypeId::Create<T>(), count)
	{}

	template <typename T>
	TypeVector::TypeVector(size_t count, const T& value)
		: TypeVector(TypeId::Create<T>(), count, &value)
	{}

	template <typename T>
	TypeVector::TypeVector(std::initializer_list<T> initializer)
		: m_ContainedType{ TypeId::Create<T>() }
		, m_ElementSize{ sizeof(T) }
	{
		assert(AssertType(TypeId::Create<T>));

		ResizeUninitialized(initializer.size());

		for (size_t i{}; i < initializer.size(); ++i)
		{
			*static_cast<T*>((*this)[i]) = initializer[i];
		}
	}

	inline TypeVector& TypeVector::operator=(const TypeVector& other)
	{
		if (&other != this)
		{
			m_ContainedType = other.m_ContainedType;
			m_ElementSize = other.m_ElementSize;

			ResizeUninitialized(other.m_Size);

			const auto& info = m_ContainedType.GetInfo();
			const auto& copyConstructor = info.CopyConstructor;

			for (size_t i{}; i < Size(); ++i)
			{
				copyConstructor((*this)[i], other[i]);
			}
		}
		return *this;
	}

	inline TypeVector& TypeVector::operator=(TypeVector&& other) noexcept
	{
		if (&other != this)
		{
			m_ContainedType		= other.m_ContainedType;
			m_Data				= std::move(other.m_Data);
			m_Capacity			= other.m_Capacity;
			m_Size				= other.m_Size;
			m_ElementSize		= other.m_ElementSize;

			other.m_Capacity	= 0;
			other.m_Size		= 0;
		}
		return *this;
	}

	inline const void* TypeVector::At(size_t index) const
	{
		if (index < Size())
			throw std::out_of_range("index out of range");

		return m_Data.get() + index * m_ElementSize;
	}

	inline void* TypeVector::At(size_t index)
	{
		if (index < Size())
			throw std::out_of_range("index out of range");

		return m_Data.get() + index * m_ElementSize;
	}

	inline const void* TypeVector::operator[](size_t index) const
	{
		assert(index < Size());
		return m_Data.get() + index * m_ElementSize;
	}

	inline void* TypeVector::operator[](size_t index)
	{
		assert(index < Size());
		return m_Data.get() + index * m_ElementSize;
	}

	inline void TypeVector::ShrinkToFit()
	{
		if (Capacity() > Size())
		{
			auto newBuffer = CreateNewBuffer(Size());
			MoveToNewBuffer(newBuffer);
			SetBuffer(std::move(newBuffer), Size());
		}
	}

	inline void TypeVector::Reserve(size_t size)
	{
		if (size > Capacity())
		{
			auto newBuffer = CreateNewBuffer(size);
			MoveToNewBuffer(newBuffer);
			SetBuffer(std::move(newBuffer), size);
		}
	}

	inline void* TypeVector::PushBackCopy(void* data)
	{
		if (Size() >= Capacity())
		{
			ResizeZeroed(CalculateNewSize());
		}

		const auto& info = m_ContainedType.GetInfo();
		const auto& copyConstructor = info.CopyConstructor;

		++m_Size;
		void* back = Back();

		copyConstructor(back, data);
		return back;
	}

	inline void* TypeVector::PushBackMove(void* data)
	{
		if (Size() >= Capacity())
		{
			ResizeZeroed(CalculateNewSize());
		}

		const auto& info = m_ContainedType.GetInfo();
		const auto& moveConstructor = info.MoveConstructor;

		++m_Size;
		void* back = Back();

		moveConstructor(back, data);
		return back;
	}

	inline void TypeVector::PopBack()
	{
		assert(Size() > 0);

		const auto& info = m_ContainedType.GetInfo();
		const auto& destructor = info.Destructor;

		destructor(Back());
		--m_Size;
	}

	inline void TypeVector::PopBack(size_t amount)
	{
		assert(Size() - amount > 0);

		const auto& info = m_ContainedType.GetInfo();
		const auto& destructor = info.Destructor;

		for (size_t i{ Size() - amount }; i < Size(); ++i)
		{
			destructor((*this)[i]);
		}
		m_Size -= amount;
	}

	inline void TypeVector::Clear()
	{
		const auto& info = m_ContainedType.GetInfo();
		const auto& destructor = info.Destructor;

		for (auto& element : *this)
		{
			destructor(element);
		}
		m_Size = 0;
	}

	inline void TypeVector::Resize(size_t size)
	{
		const auto& info = m_ContainedType.GetInfo();
		const auto& constructor = info.Constructor;

		const size_t originalSize = Size();

		ResizeUninitialized(size);

		for (size_t i{ originalSize }; i < size; ++i)
		{
			constructor((*this)[i]);
		}
	}

	inline void TypeVector::ResizeZeroed(size_t size)
	{
		const size_t originalSize = Size();
		ResizeUninitialized(size);

		std::memset((*this)[originalSize], 0, (size - originalSize) * ElementSize());
	}

	inline void TypeVector::ResizeUninitialized(size_t size)
	{
		if (size == Size())
			return;

		if (size < Size())
		{
			PopBack(Size() - size);
		}

		auto newBuffer = CreateNewBuffer(size);
		MoveToNewBuffer(newBuffer);
		SetBuffer(std::move(newBuffer), size);
	}

	inline void TypeVector::SwapRemove(size_t )
	{

	}

	inline bool TypeVector::AssertType(TypeId id)
	{
		auto& info = id.GetInfo();
		return info.MoveConstructor && info.Constructor;
	}

	inline void TypeVector::MoveToNewBuffer(std::unique_ptr<uint8_t[]>& buffer)
	{
		const auto& info = m_ContainedType.GetInfo();
		const auto& moveConstructor = info.MoveConstructor;

		for (size_t i{}; i < Size(); ++i)
		{
			const size_t elementOffset = i + m_ElementSize;
			moveConstructor(buffer.get() + elementOffset, Data() + elementOffset);
		}
	}

	inline std::unique_ptr<uint8_t[]> TypeVector::CreateNewBuffer(size_t size) const
	{
		return std::make_unique<uint8_t[]>(size * m_ElementSize);
	}

	inline void TypeVector::SetBuffer(std::unique_ptr<uint8_t[]>&& buffer, size_t elementAmount)
	{
		m_Data = std::move(buffer);
		m_Capacity = elementAmount;
	}

	constexpr size_t TypeVector::CalculateNewSize() const
	{
		return (Size() + 1) * 3 / 2;
	}
}
