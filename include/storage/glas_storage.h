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

		if constexpr (std::is_copy_constructible_v<T>)
			info.CopyConstructor = [](void* location, void* other)
		{
			new (location) T(*static_cast<T*>(other));
		};

		if constexpr (std::is_move_constructible_v<T>)
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
	T* TypeStorage::As()
	{
		return (TypeId::Create<T>() == TypeId) ? static_cast<T*>(GetData()) : nullptr;
	}

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

	inline SharedTypeStorage WeakTypeStorage::GetSharedStorage() const
	{
		SharedTypeStorage returnVal = { TypeId };
		if (!Expired())
		{
			returnVal.Data = Data.lock();
		}

		return returnVal;
	}

	template <typename T>
	T* SharedTypeStorage::As()
	{
		return (TypeId::Create<T>() == TypeId) ? static_cast<T*>(GetData()) : nullptr;
	}
}
