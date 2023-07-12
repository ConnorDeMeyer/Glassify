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
	concept Swappable = requires(T & lhs, T & rhs) { std::swap(lhs, rhs); };

	template <typename T>
	inline constexpr bool EnableCopyConstructor = std::is_copy_constructible_v<T>;

	template <typename T>
	inline constexpr bool EnableMoveConstructor = std::is_move_constructible_v<T>;

#define GLAS_STORAGE_DISABLE_COPY(TYPE) template <> inline constexpr bool glas::Storage::EnableCopyConstructor<TYPE> = false;
#define GLAS_STORAGE_DISABLE_MOVE(TYPE) template <> inline constexpr bool glas::Storage::EnableMoveConstructor<TYPE> = false;

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

		TypeId GetType() const { return m_TypeId; }
		void* GetData() const { return m_Data.get(); }

		template <typename T>
		T* As() const;

	private:
		std::unique_ptr<uint8_t[]> m_Data{};
		TypeId m_TypeId{};
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

		TypeId GetType() const { return m_TypeId; }
		void* GetData() const { return m_Data.get(); }

		template <typename T>
		T* As();
	private:
		std::shared_ptr<uint8_t[]> m_Data{};
		TypeId m_TypeId{};
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
		TypeId GetType() const { return m_TypeId; }
		void* GetData() const { return m_Data.lock().get(); }
		bool Expired() const { return m_Data.expired(); }

	private:
		std::weak_ptr<uint8_t[]> m_Data{};
		TypeId m_TypeId{};
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


		constexpr size_t	GetJumpTableSize		()	const	{ return m_Size * sizeof(void*); }
		constexpr size_t	GetVariableIdsSize		()	const	{ return m_Size * sizeof(VariableId); }

		constexpr size_t	GetJumpTableOffset		()	const	{ return 0; }
		constexpr size_t	GetVariableIdsOffset	()	const	{ return GetJumpTableOffset() + GetJumpTableSize(); }
		constexpr size_t	GetVariableDataOffset	()	const	{ return GetVariableIdsOffset() + GetVariableIdsSize(); }

		const void*			GetJumpTablePtr			()	const	{ return m_Data.get(); }
		const VariableId*	GetVariableIdsPtr		()	const	{ return reinterpret_cast<VariableId*>(m_Data.get() + GetVariableIdsOffset()); }
		const void*			GetVariableDataPtr		()	const	{ return m_Data.get() + GetVariableDataOffset(); }

		std::span<VariableId> GetVariableIds		()			{ return { GetVariableIdsPtr(), m_Size }; }
		void*				GetJumpTablePtr			()			{ return m_Data.get(); }
		VariableId*			GetVariableIdsPtr		()			{ return reinterpret_cast<VariableId*>(m_Data.get() + GetVariableIdsOffset()); }
		void*				GetVariableDataPtr		()			{ return m_Data.get() + GetVariableDataOffset(); }

		std::span<const VariableId> GetVariableIds	()	const	{ return { GetVariableIdsPtr(), m_Size }; }

		constexpr uint32_t	GetSize					()	const	{ return m_Size; }

	private:
		void Initialize(std::span<VariableId> variables, bool InitializeToDefault);

		uint32_t CalculateAlignment(std::span<VariableId> variables);

	private:
		std::unique_ptr<uint8_t[]> m_Data{};
		uint32_t m_Size{};
	};

	template <typename ValueType = void*>
	class TypeIteratorBase
	{
	public:
		using iterator_category		= std::contiguous_iterator_tag;
		using value_type			= ValueType;
		using difference_type		= size_t;
		using pointer				= ValueType;
		using reference				= ValueType&;
	public:
		TypeIteratorBase(const TypeIteratorBase&) = default;
		TypeIteratorBase(TypeIteratorBase&&) = default;
		TypeIteratorBase& operator=(const TypeIteratorBase&) = default;
		TypeIteratorBase& operator=(TypeIteratorBase&&) = default;
		~TypeIteratorBase() = default;
		TypeIteratorBase(pointer ptr, size_t variableSize) : m_Ptr(ptr), m_ElementSize(variableSize) {}

		template <typename T>
		TypeIteratorBase(T* ptr) : m_Ptr(ptr), m_ElementSize(sizeof(T)) {}
	public:

		reference operator*() { return m_Ptr; }
		value_type operator*() const { return m_Ptr; }
		reference operator->() { return m_Ptr; }

		value_type operator[](size_t offset) const { return static_cast<void*>(BytePtr() + offset * m_ElementSize); }
	public:

		template <typename T>
		T* get() { return static_cast<T*>(m_Ptr); }

	protected:

		uint8_t* BytePtr() const { return static_cast<uint8_t*>(m_Ptr); }

	protected:
		pointer m_Ptr{};
		size_t m_ElementSize{};
	};

	template <typename ValueType = void*>
	class ForwardTypeIteratorBase final : public TypeIteratorBase<ValueType>
	{
		using Super = TypeIteratorBase<ValueType>;
	public:
		ForwardTypeIteratorBase(const ForwardTypeIteratorBase&) = default;
		ForwardTypeIteratorBase(ForwardTypeIteratorBase&&) = default;
		ForwardTypeIteratorBase& operator=(const ForwardTypeIteratorBase&) = default;
		ForwardTypeIteratorBase& operator=(ForwardTypeIteratorBase&&) = default;
		~ForwardTypeIteratorBase() = default;
		ForwardTypeIteratorBase(typename Super::pointer ptr, size_t variableSize) : Super(ptr, variableSize) {}

		template <typename T>
		ForwardTypeIteratorBase(T* ptr) : Super(ptr) {}
	public:
		bool operator==	(const ForwardTypeIteratorBase& rhs) const { return Super::m_Ptr == rhs.m_Ptr; }
		bool operator!=	(const ForwardTypeIteratorBase& rhs) const { return Super::m_Ptr != rhs.m_Ptr; }
		bool operator<	(const ForwardTypeIteratorBase& rhs) const { return Super::m_Ptr < rhs.m_Ptr; }
		bool operator>	(const ForwardTypeIteratorBase& rhs) const { return Super::m_Ptr > rhs.m_Ptr; }
		bool operator<=	(const ForwardTypeIteratorBase& rhs) const { return Super::m_Ptr <= rhs.m_Ptr; }
		bool operator>=	(const ForwardTypeIteratorBase& rhs) const { return Super::m_Ptr >= rhs.m_Ptr; }

		ForwardTypeIteratorBase& operator++() { Super::m_Ptr = Super::BytePtr() + Super::m_ElementSize; return *this; }
		ForwardTypeIteratorBase& operator--() { Super::m_Ptr = Super::BytePtr() - Super::m_ElementSize; return *this; }
		ForwardTypeIteratorBase operator++(int) { ForwardTypeIteratorBase ph{ *this }; Super::m_Ptr = Super::BytePtr() + Super::m_ElementSize; return ph; }
		ForwardTypeIteratorBase operator--(int) { ForwardTypeIteratorBase ph{ *this }; Super::m_Ptr = Super::BytePtr() - Super::m_ElementSize; return ph; }

		ForwardTypeIteratorBase& operator+=(size_t rhs) { Super::m_Ptr = Super::BytePtr() + rhs * Super::m_ElementSize; return *this; }
		ForwardTypeIteratorBase& operator-=(size_t rhs) { Super::m_Ptr = Super::BytePtr() - rhs * Super::m_ElementSize; return *this; }

		ForwardTypeIteratorBase operator+(size_t rhs) { return ForwardTypeIteratorBase{ *this += rhs }; }
		ForwardTypeIteratorBase operator-(size_t rhs) { return ForwardTypeIteratorBase{ *this -= rhs }; }
	};

	template <typename ValueType = void*>
	class ReverseTypeIteratorBase final : public TypeIteratorBase<ValueType>
	{
		using Super = TypeIteratorBase<ValueType>;
	public:
		ReverseTypeIteratorBase(const ReverseTypeIteratorBase&) = default;
		ReverseTypeIteratorBase(ReverseTypeIteratorBase&&) = default;
		ReverseTypeIteratorBase& operator=(const ReverseTypeIteratorBase&) = default;
		ReverseTypeIteratorBase& operator=(ReverseTypeIteratorBase&&) = default;
		~ReverseTypeIteratorBase() = default;
		ReverseTypeIteratorBase(typename Super::pointer ptr, size_t variableSize) : Super(ptr, variableSize) {}

		template <typename T>
		ReverseTypeIteratorBase(T* ptr) : Super(ptr) {}
	public:
		bool operator==	(const ReverseTypeIteratorBase& rhs) const { return Super::m_Ptr == rhs.m_Ptr; }
		bool operator!=	(const ReverseTypeIteratorBase& rhs) const { return Super::m_Ptr != rhs.m_Ptr; }
		bool operator<	(const ReverseTypeIteratorBase& rhs) const { return Super::m_Ptr > rhs.m_Ptr; }
		bool operator>	(const ReverseTypeIteratorBase& rhs) const { return Super::m_Ptr < rhs.m_Ptr; }
		bool operator<=	(const ReverseTypeIteratorBase& rhs) const { return Super::m_Ptr >= rhs.m_Ptr; }
		bool operator>=	(const ReverseTypeIteratorBase& rhs) const { return Super::m_Ptr <= rhs.m_Ptr; }

		ReverseTypeIteratorBase& operator++() { Super::m_Ptr = Super::BytePtr() - Super::m_ElementSize; return *this; }
		ReverseTypeIteratorBase& operator--() { Super::m_Ptr = Super::BytePtr() + Super::m_ElementSize; return *this; }
		ReverseTypeIteratorBase operator++(int) { ReverseTypeIteratorBase ph{ *this }; Super::m_Ptr = Super::BytePtr() - Super::m_ElementSize; return ph; }
		ReverseTypeIteratorBase operator--(int) { ReverseTypeIteratorBase ph{ *this }; Super::m_Ptr = Super::BytePtr() + Super::m_ElementSize; return ph; }

		ReverseTypeIteratorBase& operator+=(size_t rhs) { Super::m_Ptr = Super::BytePtr() - rhs * Super::m_ElementSize; return *this; }
		ReverseTypeIteratorBase& operator-=(size_t rhs) { Super::m_Ptr = Super::BytePtr() + rhs * Super::m_ElementSize; return *this; }

		ReverseTypeIteratorBase operator+(size_t rhs) { return ReverseTypeIteratorBase{ *this += rhs }; }
		ReverseTypeIteratorBase operator-(size_t rhs) { return ReverseTypeIteratorBase{ *this -= rhs }; }
	};

	using TypeIterator				= ForwardTypeIteratorBase<void*>;
	using ConstTypeIterator			= ForwardTypeIteratorBase<const void*>;
	using ReverseTypeIterator		= ReverseTypeIteratorBase<void*>;
	using ReverseConstTypeIterator	= ReverseTypeIteratorBase<const void*>;

	class TypeVector final
	{
	public:
		TypeVector() = default;
		~TypeVector();

		TypeVector(const TypeVector& other);
		TypeVector(TypeVector&& other) noexcept;

		TypeVector(TypeId type, size_t count);
		TypeVector(size_t count, TypeStorage& value);
		TypeVector(TypeId type, size_t count, const void* value);

		template <typename T> TypeVector(size_t count);
		template <typename T> TypeVector(size_t count, const T& value);

		template <typename T> TypeVector(std::initializer_list<T> initializer);

		TypeVector& operator=(const TypeVector& other);
		TypeVector& operator=(TypeVector&& other) noexcept;
	public:

		constexpr TypeId			GetType			()				const	{ return m_ContainedType; }
		constexpr uint32_t			ElementSize		()				const	{ return m_ElementSize; }

	public: // Element access

		void*						At				(size_t index);
		const void*					At				(size_t index)	const;
		void*						operator[]		(size_t index);
		const void*					operator[]		(size_t index)	const;
		uint8_t*					Data			()						{ return m_Data.get(); }
		const uint8_t*				Data			()				const	{ return m_Data.get(); }
		void*						Front			()						{ return Data(); }
		const void*					Front			()				const	{ return Data(); }
		void*						Back			()						{ return Data() + ElementSize() * (Size() - 1); }
		const void*					Back			()				const	{ return Data() + ElementSize() * (Size() - 1); }

	public: // Iterators

		TypeIterator				begin		()	noexcept		{ return { Front(), ElementSize() }; }
		TypeIterator				end			()	noexcept		{ return { Data() + Size() * ElementSize(), ElementSize()}; }
		ConstTypeIterator			begin		()	const noexcept	{ return { Front(), ElementSize() }; }
		ConstTypeIterator			end			()	const noexcept	{ return { Data() + Size() * ElementSize(), ElementSize()}; }
		ReverseTypeIterator			rbegin		()	noexcept		{ return { Back(), ElementSize() }; }
		ReverseTypeIterator			rend		()	noexcept		{ return { Data() - ElementSize(), ElementSize() }; }
		ReverseConstTypeIterator	rbegin		()	const noexcept	{ return { Back(), ElementSize() }; }
		ReverseConstTypeIterator	rend		()	const noexcept	{ return { Data() - ElementSize(), ElementSize() }; }

	public:	// Capacity

		constexpr bool				IsEmpty			()				const	{ return Size() == 0; }
		constexpr size_t			Size			()				const	{ return m_Size; }
		constexpr size_t			Capacity		()				const	{ return m_Capacity; }
		void						Reserve			(size_t size);
		void						ShrinkToFit		();

	public: // Modifiers

		void*						PushBackCopy			(void* data);
		void*						PushBackMove			(void* data);
		void						PopBack					();
		void						PopBack					(size_t amount);
		void						Clear					();
		void						Resize					(size_t size);
		void						ResizeZeroed			(size_t size);
		void						ResizeUninitialized		(size_t size);
		void						SwapRemove				(size_t index);

	private:
		bool						AssertType		(TypeId id);
		void						MoveToNewBuffer	(std::unique_ptr<uint8_t[]>& buffer);
		std::unique_ptr<uint8_t[]>	CreateNewBuffer	(size_t size)	const;
		void						SetBuffer		(std::unique_ptr<uint8_t[]>&& buffer, size_t elementAmount);
		constexpr size_t			CalculateNewSize()				const;

	private:
		TypeId						m_ContainedType	{ };
		std::unique_ptr<uint8_t[]>	m_Data			{ };
		size_t						m_Size			{ };
		size_t						m_Capacity		{ };
		uint32_t					m_ElementSize	{ };
	};
}
