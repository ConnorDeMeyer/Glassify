﻿
#include <random>
#include <utility>

#include "glassify.h"

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch2/catch.hpp"

namespace StorageTest
{


#pragma warning(disable:4324) // disable padding warning
	struct alignas(16) Vector
	{
		float X{}, Y{}, Z{};

		Vector() = default;
		Vector(float x, float y, float z) : X{ x }, Y{ y }, Z{ z } {}
	};

	GLAS_TYPE(Vector);

	GLAS_MEMBER(Vector, X);
	GLAS_MEMBER(Vector, Y);
	GLAS_MEMBER(Vector, Z);

	struct alignas(16) Quaternion
	{
		float X{}, Y{}, Z{}, W{};
	};

	GLAS_MEMBER(Quaternion, X);
	GLAS_MEMBER(Quaternion, Y);
	GLAS_MEMBER(Quaternion, Z);
	GLAS_MEMBER(Quaternion, W);

	struct Transform
	{
		Vector Translation{};
		Quaternion Rotation{};
		Vector Scale{ 1, 1, 1 };
	};
#pragma warning(default:4324)

	GLAS_MEMBER(Transform, Translation);
	GLAS_MEMBER(Transform, Rotation);
	GLAS_MEMBER(Transform, Scale);

	class GameObject final
	{
	public:
		auto& GetTransform() { return GlobalTransform; }
		auto& GetName() { return Name; }
		auto& GetId() { return Id; }
		GameObject& Randomize();

		void SetName(std::string name) { Name = std::move(name); }
	private:
		Transform GlobalTransform{};
		std::string Name{ "None" };
		uint32_t Id{};

		friend struct RegisterGameObject;
	};
}

GLAS_PRIVATE_MEMBER(StorageTest::GameObject, Name);
GLAS_PRIVATE_MEMBER(StorageTest::GameObject, Id);
GLAS_PRIVATE_MEMBER(StorageTest::GameObject, GlobalTransform);

namespace StorageTest
{
	class Scene final
	{
	public:
		auto& GetName() { return Name; }
		auto& GetOjects() { return Objects; }
		auto& GetMap() { return ObjectsMap; }
	private:
		std::string Name{};
		std::vector<GameObject> Objects{};
		std::unordered_map<int, GameObject> ObjectsMap{};

		friend struct RegisterScene;
	};
}

GLAS_PRIVATE_MEMBER(StorageTest::Scene, Name);
GLAS_PRIVATE_MEMBER(StorageTest::Scene, Objects);
GLAS_PRIVATE_MEMBER(StorageTest::Scene, ObjectsMap);

namespace StorageTest
{
	struct TestClass
	{
		std::array<int, 6> Array{};
		std::set<int> Set{};
		std::unordered_set<int> UnSet{};
		std::map<int, int> Map{};
		std::unordered_map<int, int> UnMap{};
		std::deque<int> Deque{};
		std::list<int> List{};
		std::forward_list<int> ForList{};
	};

	GLAS_MEMBER(TestClass, Array);
	GLAS_MEMBER(TestClass, Set);
	GLAS_MEMBER(TestClass, UnSet);
	GLAS_MEMBER(TestClass, Map);
	GLAS_MEMBER(TestClass, UnMap);
	GLAS_MEMBER(TestClass, Deque);
	GLAS_MEMBER(TestClass, List);
	GLAS_MEMBER(TestClass, ForList);


	std::random_device g_RandomDevice;
	std::default_random_engine g_Engine(g_RandomDevice());

	GameObject& GameObject::Randomize()
	{
		std::uniform_real_distribution<float> distribution(-100.f, 100.f);
		GlobalTransform.Rotation.X = distribution(g_Engine);
		GlobalTransform.Rotation.Y = distribution(g_Engine);
		GlobalTransform.Rotation.Z = distribution(g_Engine);
		GlobalTransform.Rotation.W = distribution(g_Engine);
		GlobalTransform.Scale.X = distribution(g_Engine);
		GlobalTransform.Scale.Y = distribution(g_Engine);
		GlobalTransform.Scale.Z = distribution(g_Engine);
		GlobalTransform.Translation.X = distribution(g_Engine);
		GlobalTransform.Translation.Y = distribution(g_Engine);
		GlobalTransform.Translation.Z = distribution(g_Engine);

		std::uniform_int_distribution<uint32_t> intDistribution{};
		Id = intDistribution(g_Engine);

		Name.clear();
		Name.reserve(10);
		std::uniform_int_distribution<int> charDistribution('a', 'z');
		for (size_t i{}; i < 10; ++i)
		{
			Name += static_cast<char>(charDistribution(g_Engine));
		}

		return *this;
	}

	struct CleanupTester
	{
		CleanupTester(bool& aliveCheck) : IsAlive{ &aliveCheck } { assert(aliveCheck == false); *IsAlive = true; }

		~CleanupTester()
		{
			if (IsAlive)
			{
				assert(*IsAlive == true);
				*IsAlive = false;
			}
		}

		CleanupTester(CleanupTester&& other) noexcept
			: IsAlive{ other.IsAlive }
		{
			other.IsAlive = nullptr;
		}

		CleanupTester& operator=(CleanupTester&& other) noexcept
		{
			std::swap(IsAlive, other.IsAlive);
			return *this;
		}

		bool* IsAlive;
	};

	GLAS_TYPE(CleanupTester);

	struct NonCopyableType
	{
		NonCopyableType() = default;
		NonCopyableType(const NonCopyableType&) = delete;
		NonCopyableType(NonCopyableType&&) noexcept = default;
		NonCopyableType& operator=(const NonCopyableType&) = delete;
		NonCopyableType& operator=(NonCopyableType&&) noexcept = default;

		NonCopyableType(int, int&, const int&, int&&) {}
	};

	struct VerboseClass
	{
		static constexpr std::string_view ConstructionMessage = "Constructed";
		static constexpr std::string_view DestructionMessage = "Destructed";
		static constexpr std::string_view CopyConstructionMessage = "Copy Constructed";
		static constexpr std::string_view MoveConstructionMessage = "Move Constructed";

		VerboseClass() { PrintMessage(ConstructionMessage); }
		~VerboseClass() { PrintMessage(DestructionMessage); }
		VerboseClass(const VerboseClass& other) { PrintMessage(CopyConstructionMessage); ID = other.ID; }
		VerboseClass(VerboseClass&& other) noexcept { PrintMessage(MoveConstructionMessage); ID = other.ID; }

		void SayHello() { std::cout << "Hello world, my ID is: " << ID << '\n'; }

		void PrintMessage(std::string_view message)
		{
			LastMessage = message;
			std::cout << LastMessage << '\n';
		}

		int ID{};
		std::string_view LastMessage;
	};

	void swap(VerboseClass& lhs, VerboseClass& rhs) noexcept
	{
		std::swap(lhs.ID, rhs.ID);
	}

	using namespace glas::Storage;

	GLAS_TYPE(VerboseClass);

	TEST_CASE("Storage Type info", "[TypeInfo]")
	{
		/**
		 * get reflection info of the VerboseClass
		 */
		auto verboseClassId = glas::TypeId::Create<VerboseClass>();
		auto info = verboseClassId.GetInfo();

		/**
		 * allocate memory for the type.
		 */
		auto data = std::make_unique<uint8_t[]>(info.Size);
		auto classInstance = reinterpret_cast<VerboseClass*>(data.get()); // not yet initialized

		/**
		 * Construct the type
		 */
		info.Constructor(data.get());
		REQUIRE(classInstance->LastMessage == VerboseClass::ConstructionMessage);

		/**
		 * Check the initialization
		 */
		REQUIRE(classInstance->ID == 0);
		classInstance->SayHello();

		/**
		 * Modify the type
		 */
		classInstance->ID = 42;
		REQUIRE(classInstance->ID == 42);
		classInstance->SayHello();

		/**
		 * Destruct the type
		 */
		info.Destructor(data.get());
		REQUIRE(classInstance->LastMessage == VerboseClass::DestructionMessage); // unsafe
	}

	TEST_CASE("Storage Type info Copy", "[TypeInfoCopy]")
	{
		/**
		 * get reflection info of the VerboseClass
		 */
		auto verboseClassId = glas::TypeId::Create<VerboseClass>();
		auto info = verboseClassId.GetInfo();

		/**
		 * allocate memory for 2 instances of the type.
		 */
		auto data = std::make_unique<uint8_t[]>(info.Size);
		auto classInstance1 = reinterpret_cast<VerboseClass*>(data.get()); // not yet initialized
		auto data2 = std::make_unique<uint8_t[]>(info.Size);
		auto classInstance2 = reinterpret_cast<VerboseClass*>(data2.get()); // not yet initialized

		/**
		 * Construct the first instance
		 */
		info.Constructor(data.get());
		REQUIRE(classInstance1->LastMessage == VerboseClass::ConstructionMessage);

		/**
		 * Modify the first instance
		 */
		classInstance1->ID = 42;
		REQUIRE(classInstance1->ID == 42);
		classInstance1->SayHello();


		/**
		 * Construct the second instance using the copy constructor to make a copy of the first instance.
		 */
		info.CopyConstructor(data2.get(), data.get());
		REQUIRE(classInstance2->LastMessage == VerboseClass::CopyConstructionMessage);

		/**
		 * Check if it is a copy
		 */
		REQUIRE(classInstance2->ID == 42);
		classInstance2->SayHello();

		/**
		 * Modify the copy
		 */
		classInstance2->ID = 12;

		/**
		 * Check if the copy has been modified and the original is still the same.
		 */
		REQUIRE(classInstance1->ID == 42);
		REQUIRE(classInstance2->ID == 12);
		classInstance1->SayHello();
		classInstance2->SayHello();


		/**
		 * Swap them both using the Swap function pointer.
		 */
		info.Swap(classInstance1, classInstance2);
		std::cout << "swapped\n";

		/**
		 * Check if they have been swapped
		 */
		REQUIRE(classInstance1->ID == 12);
		REQUIRE(classInstance2->ID == 42);
		classInstance1->SayHello();
		classInstance2->SayHello();

		/**
		 * Destruct both instances.
		 */
		info.Destructor(data.get());
		REQUIRE(classInstance1->LastMessage == VerboseClass::DestructionMessage); // unsafe

		info.Destructor(data2.get());
		REQUIRE(classInstance2->LastMessage == VerboseClass::DestructionMessage); // unsafe
	}

	//TEST_CASE("Random Type Storage", "[RandomTypeInfo]")
	//{
	//	for (size_t i{}; i < 10; ++i)
	//	{
	//		// Get all type infos in the reflection system
	//		const auto& allTypeInfo = glas::GetAllTypeInfo();

	//		// get a random one
	//		auto randomIt = allTypeInfo.begin();
	//		std::advance(randomIt, rand() % allTypeInfo.size());

	//		glas::TypeId typeId = randomIt->first;
	//		const glas::TypeInfo& TypeInfo = randomIt->second;

	//		// if no constructor or destructor, continue
	//		if (!TypeInfo.Constructor || !TypeInfo.Destructor)
	//			continue;

	//		// allocate and construct
	//		auto randomTypeInstance = std::make_unique<uint8_t[]>(TypeInfo.Size);
	//		TypeInfo.Constructor(randomTypeInstance.get());

	//		// serialize to std::cout;
	//		std::cout << "Created Type " << TypeInfo.Name << '\n';
	//		glas::Serialization::Serialize(std::cout, randomTypeInstance.get(), typeId);
	//		std::cout << "\n\n\n";

	//		// destruct
	//		TypeInfo.Destructor(randomTypeInstance.get());
	//	}
	//}

	TEST_CASE("Type Storage", "[TypeStorage]")
	{
		SECTION("As")
		{
			auto storage = TypeStorage::CopyConstruct<int>(6);
			REQUIRE(*storage.As<int>() == 6);
		}

		SECTION("Construct")
		{
			auto defaultVector = TypeStorage::Construct<Vector>();
			REQUIRE(defaultVector.As<Vector>()->X == 0);

			auto InitializedVector = TypeStorage::Construct<Vector>(4.f, 2.f, 1.f);
			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);

			{
				int test{};
				TypeStorage::Construct<NonCopyableType>(1, test, 2, 3);
			}
		}

		SECTION("Construct Type ID")
		{
			auto defaultVector = TypeStorage(glas::TypeId::Create<Vector>());
			REQUIRE(defaultVector.As<Vector>()->X == 0);
		}

		SECTION("Copy Construct")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = TypeStorage::CopyConstruct<Vector>(vectorOriginal);
			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Copy Construct Type ID")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = TypeStorage::CopyConstruct(glas::TypeId::Create<Vector>(), &vectorOriginal);

			REQUIRE(vectorOriginal.X == 4.f);
			REQUIRE(vectorOriginal.Y == 2.f);
			REQUIRE(vectorOriginal.Z == 1.f);
			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Move Construct")
		{
			{
				auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
				auto InitializedVector = TypeStorage::MoveConstruct<Vector>(std::move(vectorOriginal));

				REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
				REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
				REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
			}
			{
				auto vectorOriginal = Scene{};
				auto InitializedVector = TypeStorage::MoveConstruct<Scene>(std::move(vectorOriginal));
			}
		}

		SECTION("Move Construct ID")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = TypeStorage::MoveConstruct(glas::TypeId::Create<Vector>(), &vectorOriginal);

			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Construct Memory Management")
		{
			bool isAlive{};
			{
				auto tester = TypeStorage::Construct<CleanupTester>(isAlive);
				REQUIRE(isAlive == true);
			}
			REQUIRE(isAlive == false);
		}

		SECTION("TransferOwnershipCheck")
		{
			bool isAlive{};
			{
				auto tester = TypeStorage::Construct<CleanupTester>(isAlive);
				REQUIRE(isAlive == true);

				auto newOwner = tester.TransferOwnershipCheck<CleanupTester>();
				REQUIRE(isAlive == true);
			}
			REQUIRE(isAlive == false);

			{
				auto vectorStorage = TypeStorage::Construct<Vector>();
				REQUIRE_THROWS(vectorStorage.TransferOwnershipCheck<bool>());
			}
		}

		SECTION("TransferOwnershipUnsafe")
		{
			bool isAlive{};
			{
				auto tester = TypeStorage::Construct<CleanupTester>(isAlive);
				REQUIRE(isAlive == true);

				auto newOwner = tester.TransferOwnershipUnsafe<CleanupTester>();
				REQUIRE(isAlive == true);
			}
			REQUIRE(isAlive == false);
		}
	}

	TEST_CASE("Shared Type Storage", "[SharedTypeStorage]")
	{
		SECTION("As")
		{
			auto storage = SharedTypeStorage::CopyConstruct<int>(6);
			REQUIRE(*storage.As<int>() == 6);
		}

		SECTION("Construct")
		{
			auto defaultVector = SharedTypeStorage::Construct<Vector>();
			REQUIRE(defaultVector.As<Vector>()->X == 0);

			auto InitializedVector = SharedTypeStorage::Construct<Vector>(4.f, 2.f, 1.f);
			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Construct Type ID")
		{
			auto defaultVector = SharedTypeStorage(glas::TypeId::Create<Vector>());
			REQUIRE(defaultVector.As<Vector>()->X == 0);
		}

		SECTION("Copy Construct")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = SharedTypeStorage::CopyConstruct<Vector>(vectorOriginal);
			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Copy Construct Type ID")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = SharedTypeStorage::CopyConstruct(glas::TypeId::Create<Vector>(), &vectorOriginal);

			REQUIRE(vectorOriginal.X == 4.f);
			REQUIRE(vectorOriginal.Y == 2.f);
			REQUIRE(vectorOriginal.Z == 1.f);
			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Move Construct")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = SharedTypeStorage::MoveConstruct<Vector>(std::move(vectorOriginal));

			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Move Construct ID")
		{
			auto vectorOriginal = Vector{ 4.f, 2.f, 1.f };
			auto InitializedVector = SharedTypeStorage::MoveConstruct(glas::TypeId::Create<Vector>(), &vectorOriginal);

			REQUIRE(InitializedVector.As<Vector>()->X == 4.f);
			REQUIRE(InitializedVector.As<Vector>()->Y == 2.f);
			REQUIRE(InitializedVector.As<Vector>()->Z == 1.f);
		}

		SECTION("Construct Memory Management")
		{
			bool isAlive{};
			{
				auto tester = SharedTypeStorage::Construct<CleanupTester>(isAlive);
				REQUIRE(isAlive == true);
			}
			REQUIRE(isAlive == false);
		}

		SECTION("Sharing Memory Management")
		{
			bool isAlive{};
			{
				auto tester = SharedTypeStorage::Construct<CleanupTester>(isAlive);
				REQUIRE(isAlive == true);
				{
					auto tester2 = tester;
					REQUIRE(isAlive == true);
				}
				REQUIRE(isAlive == true);
			}
			REQUIRE(isAlive == false);
		}
	}

	TEST_CASE("Weak Type Storage", "[WeakTypeStorage]")
	{
		SECTION("Sharing Memory Management")
		{
			WeakTypeStorage invalidTester;

			bool isAlive{};
			{
				auto tester = SharedTypeStorage::Construct<CleanupTester>(isAlive);
				REQUIRE(isAlive == true);
				{
					auto weakTester = WeakTypeStorage(tester);
					REQUIRE(isAlive == true);
				}
				REQUIRE(isAlive == true);

				invalidTester = tester;
			}
			REQUIRE(isAlive == false);

			REQUIRE(invalidTester.Expired());
		}
	}

	TEST_CASE("Type Tuple", "[TypeTuple]")
	{
		SECTION("Empty Initialization")
		{
			TypeTuple tuple{};
			REQUIRE(tuple.GetJumpTablePtr() == nullptr);
			REQUIRE(tuple.GetVariableDataPtr() == nullptr);
			REQUIRE(tuple.GetVariableIdsPtr() == nullptr);
		}

		SECTION("TypeTuple::Create empty")
		{
			auto tuple = TypeTuple::Create();
			REQUIRE(tuple.GetJumpTablePtr() == nullptr);
			REQUIRE(tuple.GetVariableDataPtr() == nullptr);
			REQUIRE(tuple.GetVariableIdsPtr() == nullptr);
		}

		SECTION("TypeTuple::Create unsigned int")
		{
			auto tuple = TypeTuple::Create<uint32_t>();
			uint32_t& Int = tuple.Get<uint32_t>(0);
			Int = 0xdeadbeef;
			REQUIRE(tuple.Get<uint32_t>(0) == 0xdeadbeef);

		}

		SECTION("TypeTuple::Create GameObject")
		{
			// TODO we get segmentation exception here on Release mode

			auto tuple = TypeTuple::Create<GameObject>();
			GameObject& pGameObject = tuple.Get<GameObject>(0);
			//pGameObject->Randomize();
			pGameObject.GetTransform().Rotation.W = 5.f;
			REQUIRE(tuple.Get<GameObject>(0).GetTransform().Rotation.W == 5.f);
		}

		SECTION("TypeTuple::Create complex")
		{
			auto tuple = TypeTuple::Create<GameObject, int, double*>();

			auto& gameObject = tuple.Get<GameObject>(0);
			auto& intRef = tuple.Get<int>(1);

			intRef = 200;
			gameObject.Randomize();
			gameObject.GetTransform().Rotation.X = 5.f;

			REQUIRE(tuple.Get<GameObject>(0).GetTransform().Rotation.X == 5.f);
			REQUIRE(tuple.Get<int>(1) == 200);
			REQUIRE(tuple.Get<double*>(2) == nullptr);
		}

		SECTION("variadic constructor copy")
		{
			std::tuple stdTuple{ GameObject{}.Randomize(), int{200}, double{50} };
			auto tuple = TypeTuple{ stdTuple };

			auto& gameObject = tuple.Get<GameObject>(0);
			gameObject.GetTransform().Rotation.X = 5.f;

			REQUIRE(tuple.Get<GameObject>(0).GetTransform().Rotation.X == 5.f);
			REQUIRE(tuple.Get<int>(1) == 200);
			REQUIRE(tuple.Get<double>(2) == 50);
		}

		SECTION("variadic constructor move")
		{
			std::tuple stdTuple{ GameObject{}.Randomize(), int{200}, double{50} };
			auto tuple = TypeTuple{ std::move(stdTuple) };

			auto& gameObject = tuple.Get<GameObject>(0);
			gameObject.GetTransform().Rotation.X = 5.f;

			REQUIRE(tuple.Get<GameObject>(0).GetTransform().Rotation.X == 5.f);
			REQUIRE(tuple.Get<int>(1) == 200);
			REQUIRE(tuple.Get<double>(2) == 50);
		}

		SECTION("Memory management")
		{
			bool testBool0{ false };
			bool testBool1{ false };

			{
				auto tester = TypeTuple::Create(
					CleanupTester{ testBool0 },
					CleanupTester{ testBool1 }
				);

				REQUIRE(*tester.Get<CleanupTester>(0).IsAlive == true);
				REQUIRE(*tester.Get<CleanupTester>(1).IsAlive == true);
			}

			REQUIRE(testBool0 == false);
			REQUIRE(testBool1 == false);
		}
	}

	TEST_CASE("Type Vector", "[TypeVector]")
	{
		auto TypeVectorTester = [](TypeVector& vector)
			{
				auto initialSize = vector.Size();

				vector.PushBack();

				REQUIRE(vector.Size() == initialSize + 1);

				vector.PopBack();

				REQUIRE(vector.Size() == initialSize);

				for (size_t i{}; i < 4; ++i)
				{
					vector.PushBack();
				}

				REQUIRE_NOTHROW(vector.Front());
				REQUIRE_NOTHROW(vector.Back());

				REQUIRE_THROWS(vector.At(1000000));

				vector.ShrinkToFit();

				vector.Reserve(100);
				REQUIRE(vector.Capacity() == 100);

				vector.SwapRemove(2);
			};

		SECTION("Normal Construction")
		{
			auto vector = TypeVector{ glas::TypeId::Create<GameObject>() };

			TypeVectorTester(vector);
		}

		SECTION("Normal Construction count")
		{
			auto vector = TypeVector{ glas::TypeId::Create<GameObject>(), 5 };

			TypeVectorTester(vector);
		}

		SECTION("Construction Type Storage")
		{
			TypeStorage storage = TypeStorage::Construct<Vector>();

			auto vector = TypeVector{ 10, storage };

			TypeVectorTester(vector);
		}

		SECTION("Construction void* value")
		{
			Vector value{ 1,2,3 };
			auto vector = TypeVector{ glas::TypeId::Create<Vector>(), 20, &value };

			TypeVectorTester(vector);
		}

		SECTION("Create Template")
		{
			auto vector = TypeVector::Create<Scene>();

			TypeVectorTester(vector);
		}

		SECTION("Create Template Count")
		{
			auto vector = TypeVector::Create<Scene>();

			TypeVectorTester(vector);
		}

		SECTION("Create Template Count value")
		{
			auto vector = TypeVector(10, Vector{ 6,2,7 });

			TypeVectorTester(vector);
		}

		SECTION("Iteration")
		{
			auto vector = TypeVector::Create<GameObject>(10);

			int counter{};
			for (auto element : vector)
			{
				static_cast<GameObject*>(element)->SetName(std::to_string(++counter));
			}

			for (auto it = vector.rbegin(); it != vector.rend(); ++it)
			{
				REQUIRE(it.get<GameObject>()->GetName() == std::to_string(counter--));
			}
		}
	}


}