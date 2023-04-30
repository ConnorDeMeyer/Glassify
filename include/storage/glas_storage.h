#pragma once

#include "glas_storage_config.h"
#include "../glas_config.h"

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
			info.CopyConstructor = [](void* location, void* other)
		{
			new (location) T(*static_cast<T*>(other));
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
		: TypeId{ id }
	{
		auto& info = id.GetInfo();
		Data = std::make_unique<uint8_t[]>(info.Size);
		assert(info.Constructor && "Type has no default constructor");
		info.Constructor(Data.get());
	}

	inline TypeStorage::~TypeStorage()
	{
		if (TypeId.GetId() && Data)
		{
			auto& info = TypeId.GetInfo();
			if (info.Destructor)
			{
				info.Destructor(Data.get());
			}
		}
	}

	inline TypeStorage::TypeStorage(TypeStorage&& other) noexcept
		: Data{ std::move(other.Data) }
		, TypeId{ other.TypeId }
	{
		other.TypeId = {};
	}

	inline TypeStorage& TypeStorage::operator=(TypeStorage&& other) noexcept
	{
		if (this != &other)
		{
			Data = std::move(other.Data);
			TypeId = other.TypeId;

			other.TypeId = {};
		}
		return *this;
	}

	template <typename T>
	TypeStorage TypeStorage::Construct() requires std::is_default_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.TypeId = TypeId::Create<T>();
		storage.Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.Data.get()) T();

		return storage;
	}

	template <typename T>
	TypeStorage TypeStorage::CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.TypeId = TypeId::Create<T>();
		storage.Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.Data.get()) T(value);

		return storage;
	}

	template <typename T>
	TypeStorage TypeStorage::MoveConstruct(T&& value) requires std::is_move_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		TypeStorage storage;
		storage.TypeId = TypeId::Create<T>();
		storage.Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.Data.get()) T(std::move(value));

		return storage;
	}

	inline TypeStorage TypeStorage::CopyConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.CopyConstructor);

		TypeStorage storage;

		storage.TypeId = id;
		storage.Data = std::make_unique<uint8_t[]>(info.Size);

		info.CopyConstructor(storage.GetData(), original);
		return storage;
	}

	inline TypeStorage TypeStorage::MoveConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.MoveConstructor);

		TypeStorage storage;

		storage.TypeId = id;
		storage.Data = std::make_unique<uint8_t[]>(info.Size);

		info.MoveConstructor(storage.GetData(), original);
		return storage;
	}

	template <typename T>
	T* TypeStorage::As() const
	{
		return (TypeId::Create<T>() == TypeId) ? static_cast<T*>(GetData()) : nullptr;
	}

	/**
	 * SHARED TYPE STORAGE
	 */

	inline SharedTypeStorage::SharedTypeStorage(glas::TypeId id)
		: TypeId{ id }
	{
		auto& info = id.GetInfo();
		Data = std::make_shared<uint8_t[]>(info.Size);
		assert(info.Constructor && "Type has no default constructor");
		info.Constructor(Data.get());
	}

	inline SharedTypeStorage::~SharedTypeStorage()
	{
		if (TypeId.GetId() && Data.use_count() == 1)
		{
			auto& info = TypeId.GetInfo();
			if (info.Destructor)
			{
				info.Destructor(Data.get());
			}
		}
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::Construct() requires std::is_default_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.TypeId = TypeId::Create<T>();
		storage.Data = std::make_shared<uint8_t[]>(sizeof(T));

		new (storage.Data.get()) T();

		return storage;
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.TypeId = TypeId::Create<T>();
		storage.Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.Data.get()) T(value);

		return storage;
	}

	template <typename T>
	SharedTypeStorage SharedTypeStorage::MoveConstruct(T&& value) requires std::is_move_constructible_v<T>
	{
		AutoRegisterTypeOnce<T> RegisterType{};

		SharedTypeStorage storage;
		storage.TypeId = TypeId::Create<T>();
		storage.Data = std::make_unique<uint8_t[]>(sizeof(T));

		new (storage.Data.get()) T(std::move(value));

		return storage;
	}

	inline SharedTypeStorage SharedTypeStorage::CopyConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.CopyConstructor);

		SharedTypeStorage storage;

		storage.TypeId = id;
		storage.Data = std::make_unique<uint8_t[]>(info.Size);

		info.CopyConstructor(storage.GetData(), original);
		return storage;
	}

	inline SharedTypeStorage SharedTypeStorage::MoveConstruct(glas::TypeId id, void* original)
	{
		auto& info = id.GetInfo();
		assert(info.MoveConstructor);

		SharedTypeStorage storage;

		storage.TypeId = id;
		storage.Data = std::make_unique<uint8_t[]>(info.Size);

		info.MoveConstructor(storage.GetData(), original);
		return storage;
	}

	inline WeakTypeStorage::WeakTypeStorage(const SharedTypeStorage& sharedStorage)
		: Data{ sharedStorage.Data }
		, TypeId{ sharedStorage.TypeId }
	{

	}

	template <typename T>
	T* SharedTypeStorage::As()
	{
		return (TypeId::Create<T>() == TypeId) ? static_cast<T*>(GetData()) : nullptr;
	}

	/**
	 * WEAK TYPE STORAGE
	 */

	inline SharedTypeStorage WeakTypeStorage::GetSharedStorage() const
	{
		SharedTypeStorage returnVal = { TypeId };
		if (!Expired())
		{
			returnVal.Data = Data.lock();
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
	 * Data Section
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
		Size = static_cast<uint32_t>(variables.size());
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
		Data = std::make_unique<uint8_t[]>(DataSize + jumpTableSize + variableIdsSize);

		// Copy the variable Ids to the allocated variable id section
		std::ranges::copy(variables, GetVariableIds().begin());

		// Populate the jump table
		uint32_t accumulatedOffset{};
		for (uint32_t i{}; i < variables.size(); ++i)
		{
			auto& var = variables[i];
			accumulatedOffset = GetOffset(structAlignment, var.GetAlign(), accumulatedOffset);

			reinterpret_cast<void**>(Data.get())[i] = VoidOffset(GetVariableDataPtr(), accumulatedOffset);

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
		return reinterpret_cast<void**>(Data.get())[index];
	}

	template <typename T>
	T& TypeTuple::Get(size_t index)
	{
		assert(VariableId::Create<T>() == GetVariableIds()[index]);
		return *static_cast<T*>(reinterpret_cast<void**>(Data.get())[index]);
	}

	template <typename T>
	const T& TypeTuple::Get(size_t index) const
	{
		assert(VariableId::Create<T>() == GetVariableIds()[index]);
		return *static_cast<T*>(reinterpret_cast<void**>(Data.get())[index]);
	}
}
