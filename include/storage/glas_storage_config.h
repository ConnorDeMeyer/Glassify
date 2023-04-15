#pragma once

#include <memory>
#include "../glas_decl.h"

namespace glas
{
	struct TypeInfo;
}

namespace glas::Storage
{
	template <typename T>
	constexpr void FillInfo(TypeInfo& info);

	class TypeStorage final
	{
	public:
		TypeStorage(TypeId id);
		TypeStorage() = default;
		~TypeStorage();

		TypeStorage(const TypeStorage& other) = delete;
		TypeStorage(TypeStorage&& other) noexcept;
		TypeStorage& operator=(const TypeStorage& other) = delete;
		TypeStorage& operator=(TypeStorage&& other) noexcept;
	public:

		template <typename T>
		static TypeStorage Construct() requires std::is_default_constructible_v<T>;

		template <typename T>
		static TypeStorage CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>;

		template <typename T>
		static TypeStorage MoveConstruct(T&& value) requires std::is_move_constructible_v<T>;

		TypeStorage static CopyConstruct(TypeId id, void* original);

		TypeStorage static MoveConstruct(TypeId id, void* original);

	public:

		TypeId GetType() const { return TypeId; }
		void* GetData() const { return Data.get(); }

		template <typename T>
		T* As() const;

	private:
		std::unique_ptr<uint8_t[]> Data{};
		TypeId TypeId{};
	};

	class SharedTypeStorage final
	{
		friend class WeakTypeStorage;
	public:
		SharedTypeStorage(TypeId id);
		SharedTypeStorage() = default;
		~SharedTypeStorage();

		SharedTypeStorage(const SharedTypeStorage& other) = default;
		SharedTypeStorage(SharedTypeStorage&& other) noexcept = default;
		SharedTypeStorage& operator=(const SharedTypeStorage& other) = default;
		SharedTypeStorage& operator=(SharedTypeStorage&& other) noexcept = default;
	public:

		template <typename T>
		static SharedTypeStorage Construct() requires std::is_default_constructible_v<T>;

		template <typename T>
		static SharedTypeStorage CopyConstruct(const T& value) requires std::is_copy_constructible_v<T>;

		template <typename T>
		static SharedTypeStorage MoveConstruct(T&& value) requires std::is_move_constructible_v<T>;

		SharedTypeStorage static CopyConstruct(TypeId id, void* original);

		SharedTypeStorage static MoveConstruct(TypeId id, void* original);

	public:

		TypeId GetType() const { return TypeId; }
		void* GetData() const { return Data.get(); }

		template <typename T>
		T* As();
	private:
		std::shared_ptr<uint8_t[]> Data{};
		TypeId TypeId{};
	};

	class WeakTypeStorage final
	{
	public:
		WeakTypeStorage(const SharedTypeStorage& sharedStorage);
		WeakTypeStorage() = default;
		~WeakTypeStorage() = default;

		WeakTypeStorage(const WeakTypeStorage& other) = default;
		WeakTypeStorage(WeakTypeStorage&& other) noexcept = default;
		WeakTypeStorage& operator=(const WeakTypeStorage& other) = default;
		WeakTypeStorage& operator=(WeakTypeStorage&& other) noexcept = default;
	public:

		SharedTypeStorage GetSharedStorage() const;
		TypeId GetType() const { return TypeId; }
		void* GetData() const { return Data.lock().get(); }
		bool Expired() const { return Data.expired(); }

	private:
		std::weak_ptr<uint8_t[]> Data{};
		TypeId TypeId{};
	};

	class TypeTuple final
	{
	public:
		TypeTuple() = default;
		~TypeTuple() = default;
		TypeTuple(std::span<VariableId> variables);

		TypeTuple(const TypeTuple& other) = delete; // TODO
		TypeTuple(TypeTuple&& other) noexcept = default;
		TypeTuple& operator=(const TypeTuple& other) = delete; // TODO
		TypeTuple& operator=(TypeTuple&& other) noexcept = default;

		template <typename... T>
		TypeTuple(const std::tuple<T...>& tuple);

		template <typename... T>
		TypeTuple(std::tuple<T...>&& tuple);

		template <typename... T>
		static TypeTuple Create();

		VariableId GetVariable(size_t index) const;

		void* GetVoid(size_t index) const;

		template <typename T>
		T& Get(size_t index);

		template <typename T>
		const T& Get(size_t index) const;


		constexpr size_t	GetJumpTableSize		()	const	{ return Size * sizeof(void*); }
		constexpr size_t	GetVariableIdsSize		()	const	{ return Size * sizeof(VariableId); }

		constexpr size_t	GetJumpTableOffset		()	const	{ return 0; }
		constexpr size_t	GetVariableIdsOffset	()	const	{ return GetJumpTableOffset() + GetJumpTableSize(); }
		constexpr size_t	GetVariableDataOffset	()	const	{ return GetVariableIdsOffset() + GetVariableIdsSize(); }

		const void*			GetJumpTablePtr			()	const	{ return Data.get(); }
		const VariableId*	GetVariableIdsPtr		()	const	{ return reinterpret_cast<VariableId*>(Data.get() + GetVariableIdsOffset()); }
		const void*			GetVariableDataPtr		()	const	{ return Data.get() + GetVariableDataOffset(); }

		std::span<VariableId> GetVariableIds		()			{ return { GetVariableIdsPtr(), Size }; }
		void*				GetJumpTablePtr			()			{ return Data.get(); }
		VariableId*			GetVariableIdsPtr		()			{ return reinterpret_cast<VariableId*>(Data.get() + GetVariableIdsOffset()); }
		void*				GetVariableDataPtr		()			{ return Data.get() + GetVariableDataOffset(); }

		std::span<const VariableId> GetVariableIds	()	const	{ return { GetVariableIdsPtr(), Size }; }

		constexpr uint32_t	GetSize					()	const	{ return Size; }

	private:
		void Initialize(std::span<VariableId> variables, bool InitializeToDefault);

		uint32_t CalculateAlignment(std::span<VariableId> variables);

	private:
		std::unique_ptr<uint8_t[]> Data{};
		uint32_t Size{};
	};
}
