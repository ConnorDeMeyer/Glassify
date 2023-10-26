#pragma once

#include <span>
#include <tuple>
#include <memory>
#include <string>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <initializer_list>

#include "glas_storage_config.h"
#include "../glas_config.h"

namespace glas::Storage
{
	template <typename T>
	constexpr void FillTypeInfo(TypeInfo& info)
	{
		if constexpr (EnableDefaultConstructor<T>)
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

		if constexpr (EnableDestructor<T>)
			info.Destructor = [](void* data)
			{
				static_cast<T*>(data)->~T();
			};

		if constexpr (EnableSwapping<T>)
			info.Swap = [](void* lhs, void* rhs)
			{
				using std::swap;
				swap(*static_cast<T*>(lhs), *static_cast<T*>(rhs));
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
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T();

		return storage;
	}

	template <typename T, typename ... Parameters>
	TypeStorage TypeStorage::Construct(Parameters&&... parameters)
	{
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T(std::forward<Parameters>(parameters)...);

		return storage;
	}

	template <typename T>
	TypeStorage TypeStorage::CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>
	{
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T(value);

		return storage;
	}

	template <typename T>
	TypeStorage TypeStorage::MoveConstruct(T&& value) requires std::is_move_constructible_v<T>
	{
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T(value);

		return storage;
	}

	inline TypeStorage TypeStorage::CopyConstruct(glas::TypeId id, const void* original)
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

	template <typename T>
	std::unique_ptr<T> TypeStorage::TransferOwnershipCheck()
	{
		if (glas::TypeId::Create<T>() != m_TypeId)
			throw std::runtime_error("Given type does not match stored type");

		m_TypeId = {};
		return std::unique_ptr<T>{reinterpret_cast<T*>(m_Data.release())};
	}

	template <typename T>
	std::unique_ptr<T> TypeStorage::TransferOwnershipUnsafe()
	{
		m_TypeId = {};
		return std::unique_ptr<T>{reinterpret_cast<T*>(m_Data.release())};
	}

	template <typename T>
	TypeStorage TypeStorage::Initialize()
	{
		GlasAutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		return storage;
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
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T();

		return storage;
	}

	template <typename T, typename ... Parameters>
	SharedTypeStorage SharedTypeStorage::Construct(Parameters&&... parameters)
	{
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T(parameters...);

		return storage;
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>
	{
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T(value);

		return storage;
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::MoveConstruct(T&& value) requires std::is_move_constructible_v<T>
	{
		auto storage = Initialize<T>();

		new (storage.m_Data.get()) T(value);

		return storage;
	}

	inline SharedTypeStorage SharedTypeStorage::CopyConstruct(glas::TypeId id, const void* original)
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

	template <typename T>
	SharedTypeStorage SharedTypeStorage::Initialize()
	{
		GlasAutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.m_TypeId = TypeId::Create<T>();
		storage.m_Data = std::make_unique<uint8_t[]>(sizeof(T));

		return storage;
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
	void InitializeDataTupleCopy(TypeTuple& typeTuple, const std::tuple<Types...>& tuple)
	{
		using Type = std::remove_reference_t<std::tuple_element_t<Index, std::tuple<Types...>>>;

		new (typeTuple.GetVoid(Index)) Type(std::get<Index>(tuple));
		//typeTuple.Get<Type>(Index) = std::get<Index>(tuple);
		if constexpr (Index + 1 < sizeof...(Types))
		{
			InitializeDataTupleCopy<Index + 1, Types...>(typeTuple, tuple);
		}
	}

	template <size_t Index, typename... Types>
	void InitializeDataTupleMove(TypeTuple& typeTuple, std::tuple<Types...>&& tuple)
	{
		using Type = std::remove_reference_t<std::tuple_element_t<Index, std::tuple<Types...>>>;

		new (typeTuple.GetVoid(Index)) Type(std::move(std::get<Index>(tuple)));
		//typeTuple.Get<Type>(Index) = std::move(std::get<Index>(tuple));
		if constexpr (Index + 1 < sizeof...(Types))
		{
			InitializeDataTupleMove<Index + 1, Types...>(typeTuple, std::move(tuple));
		}
	}

	template <size_t Index, typename T, typename ... Types>
	void InitializeDataMove(TypeTuple& typeTuple, T&& val, Types&&... vals)
	{
		using Type = std::remove_reference_t<T>;

		new (typeTuple.GetVoid(Index)) Type(std::forward<T>(val));

		if constexpr (sizeof...(Types) > 0)
		{
			InitializeDataMove<Index + 1, Types...>(typeTuple, std::forward<Types>(vals)...);
		}
	}

	inline TypeTuple::~TypeTuple()
	{
		for (uint32_t i{}; i < m_Size; ++i)
		{
			VariableId id = GetVariable(i);
			if (!id.IsRefOrPointer())
			{
				if (const auto destructor = id.GetTypeId().GetInfo().Destructor)
				{
					destructor(GetVoid(i));
				}
			}
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
		InitializeDataTupleCopy<0, T...>(*this, tuple);
	}

	template <typename ... T>
	TypeTuple::TypeTuple(std::tuple<T...>&& tuple)
	{
		auto variables = GetVariableArray<T...>();
		Initialize(variables, false);
		InitializeDataTupleMove<0, T...>(*this, std::move(tuple));
	}

	template <typename ... T>
	TypeTuple TypeTuple::Create()
	{
		//GetVariableIdArray<T...>();
		if constexpr (sizeof...(T) != 0)
		{
			auto variableArray = GetVariableArray<T...>();
			const auto variables = std::span<VariableId>(variableArray.begin(), variableArray.end());
			return { variables };
		}
		else
		{
			return {};
		}
	}

	template <typename ... T>
	TypeTuple TypeTuple::Create(T&&... val)
	{
		TypeTuple tuple;
		auto variables = GetVariableArray<T...>();
		tuple.Initialize(variables, false);

		InitializeDataMove<0, T...>(tuple, std::forward<T>(val)...);

		return tuple;
	}

	template <typename ... T>
	TypeTuple TypeTuple::CreateNoReferences()
	{
		auto variableIds = glas::GetVariableArray<T...>();
		for (auto& variable : variableIds)
		{
			variable.SetReferenceFlag(false);
		}
		return TypeTuple(variableIds);
	}

	template <typename ... T>
	TypeTuple TypeTuple::CreateNoReferences(T&&... val)
	{
		return TypeTuple(std::tuple<std::remove_reference_t<T>...>(std::forward<T>(val)...));
	}

	inline TypeTuple TypeTuple::CreateNoReferences(std::span<const VariableId> variables)
	{
		std::vector<VariableId> variableIds{ variables.begin(), variables.end() };
		for (auto& variable : variableIds)
		{
			variable.RemoveReferenceFlag();
		}
		return { std::span{ variableIds.begin(), variableIds.end() } };
	}

	inline VariableId TypeTuple::GetVariable(size_t index) const
	{
		return GetVariableIds()[index];
	}

	inline void TypeTuple::SetVariableUnsafe(size_t index, VariableId id)
	{
		GetVariableIds()[index] = id;
	}

	inline uint32_t TypeTuple::CalculateAlignment(std::span<VariableId> variables) const
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

		// Check the variables and calculate the final size of the data
		uint32_t DataSize{};
		for (auto& var : variables)
		{
			// because refs and rVal Refs cannot be stored, we store the while variable instead
			var.RemoveReferenceFlag();
			var.RemoveRValReferenceFlag();

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
		assert(VariableId::Create<T>().GetTypeId() == GetVariableIds()[index].GetTypeId());
		return *static_cast<std::remove_reference_t<T>*>(reinterpret_cast<void**>(m_Data.get())[index]);
	}

	template <typename T>
	const T& TypeTuple::Get(size_t index) const
	{
		assert(VariableId::Create<T>().GetTypeId() == GetVariableIds()[index].GetTypeId());
		return *static_cast<std::remove_reference_t<T>*>(reinterpret_cast<void**>(m_Data.get())[index]);
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

			ReserveUninitialized(other.Size());

			for (size_t i{}; i < other.Size(); ++i)
			{
				copyConstructor((*this)[i], other[i]);
			}
		}
	}

	inline TypeVector::TypeVector(TypeVector&& other) noexcept
		: m_ContainedType	{ other.m_ContainedType }
		, m_Data			{ std::move(other.m_Data )}
		, m_Size			{ other.m_Size }
		, m_Capacity		{ other.m_Capacity }
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

		ReserveUninitialized(count);
		for (auto& element : *this)
		{
			constructor(element);
		}
	}

	inline TypeVector::TypeVector(size_t count, const TypeStorage& value)
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

		ReserveUninitialized(count);
		for (auto& element : *this)
		{
			copyConstructor(element, value);
		}
	}

	inline TypeVector::TypeVector(TypeId type)
		: m_ContainedType{ type }
		, m_ElementSize{ type.GetInfo().Size }
	{
		assert(type.IsValid());
		assert(AssertType(type));
	}

	template <typename T>
	TypeVector TypeVector::Create()
	{
		return TypeVector{ TypeId::Create<T>() };
	}

	template <typename T>
	TypeVector TypeVector::Create(size_t count)
	{
		return TypeVector{ TypeId::Create<T>(), count };
	}

	template <typename T>
	TypeVector::TypeVector(size_t count, const T& value)
		: TypeVector(TypeId::Create<T>(), count, &value)
	{}

	template <typename T>
	T& TypeVector::Get(size_t index)
	{
		assert(GetType() == TypeId::Create<T>());
		return *static_cast<T*>((*this)[index]);
	}

	template <typename T>
	const T& TypeVector::Get(size_t index) const
	{
		assert(GetType() == TypeId::Create<T>());
		return *static_cast<const T*>((*this)[index]);
	}

	template <typename T>
	T& TypeVector::PushBack()
	{
		assert(GetType() == TypeId::Create<T>());
		auto address = PushBackUninitialized();

		new (address) T();

		return *static_cast<T*>(address);
	}

	template <typename T>
	T& TypeVector::PushBack(const T& element)
	{
		assert(GetType() == TypeId::Create<T>());
		auto address = PushBackUninitialized();

		new (address) T(element);

		return *static_cast<T*>(address);
	}

	template <typename T>
	T& TypeVector::PushBack(T&& element)
	{
		assert(GetType() == TypeId::Create<T>());
		auto address = PushBackUninitialized();

		new (address) T(element);

		return *static_cast<T*>(address);
	}

	//template <typename T>
	//TypeVector::TypeVector(std::initializer_list<T> initializer)
	//	: m_ContainedType{ TypeId::Create<T>() }
	//	, m_ElementSize{ sizeof(T) }
	//{
	//	assert(AssertType(TypeId::Create<T>()));
	//
	//	ReserveUninitialized(initializer.size());
	//
	//	size_t index{};
	//	for (auto& element : initializer)
	//	{
	//		*static_cast<T*>((*this)[index]) = element;
	//		++index;
	//	}
	//}

	inline TypeVector& TypeVector::operator=(const TypeVector& other)
	{
		if (&other != this)
		{
			m_ContainedType = other.m_ContainedType;
			m_ElementSize = other.m_ElementSize;

			ReserveUninitialized(other.m_Size);

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
		if (index >= Size())
			throw std::out_of_range("index out of range");

		return ElementAddress(index);
	}

	inline void* TypeVector::At(size_t index)
	{
		if (index >= Size())
			throw std::out_of_range("index out of range");

		return ElementAddress(index);
	}

	inline const void* TypeVector::operator[](size_t index) const
	{
		assert(index < Size());
		return ElementAddress(index);
	}

	inline void* TypeVector::operator[](size_t index)
	{
		assert(index < Size());
		return ElementAddress(index);
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

	inline void* TypeVector::PushBack()
	{
		if (Size() >= Capacity())
		{
			ReserveZeroed(CalculateNewSize());
		}

		const auto& info = m_ContainedType.GetInfo();
		const auto& constructor = info.Constructor;

		assert(constructor);

		++m_Size;
		void* back = Back();

		constructor(back);
		return back;
	}

	inline void* TypeVector::PushBackCopy(const void* data)
	{
		if (Size() >= Capacity())
		{
			ReserveZeroed(CalculateNewSize());
		}

		const auto& info = m_ContainedType.GetInfo();
		const auto& copyConstructor = info.CopyConstructor;

		assert(copyConstructor);

		++m_Size;
		void* back = Back();

		copyConstructor(back, data);
		return back;
	}

	inline void* TypeVector::PushBackMove(void* data)
	{
		if (Size() >= Capacity())
		{
			ReserveZeroed(CalculateNewSize());
		}

		const auto& info = m_ContainedType.GetInfo();
		const auto& moveConstructor = info.MoveConstructor;

		assert(moveConstructor);

		++m_Size;
		void* back = Back();

		moveConstructor(back, data);
		return back;
	}

	inline void* TypeVector::PushBackUninitialized()
	{
		if (Size() >= Capacity())
		{
			ReserveZeroed(CalculateNewSize());
		}

		++m_Size;

		return Back();
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

		ReserveUninitialized(size);

		m_Size = size;

		for (size_t i{ originalSize }; i < size; ++i)
		{
			constructor((*this)[i]);
		}
	}

	inline void TypeVector::ReserveZeroed(size_t size)
	{
		const size_t originalSize = Size();
		ReserveUninitialized(size);

		std::memset(ElementAddress(originalSize), 0, (size - originalSize) * ElementSize());
	}

	inline void TypeVector::ReserveUninitialized(size_t size)
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

	inline void TypeVector::SwapRemove(size_t index)
	{
		const auto& info = m_ContainedType.GetInfo();
		const auto& swap = info.Swap;

		assert(swap);

		if (index != Size() - 1)
		{
			swap((*this)[index], Back());
		}

		PopBack();
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

		assert(moveConstructor);

		for (size_t i{}; i < Size(); ++i)
		{
			const size_t elementOffset = i * m_ElementSize;
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

	inline void* TypeVector::ElementAddress(size_t index) const
	{
		return m_Data.get() + index * m_ElementSize;
	}

	constexpr size_t TypeVector::CalculateNewSize() const
	{
		return (Size() + 1) * 3 / 2;
	}
}
