
#include <random>

#include "glassify.h"

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch2/catch.hpp"

#pragma warning(disable:4324) // disable padding warning
struct alignas(16) Vector
{
	float X{}, Y{}, Z{};
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
	auto& GetTransform() { return Transform; }
	auto& GetName() { return Name; }
	auto& GetId() { return Id; }
	GameObject& Randomize();
private:
	Transform Transform{};
	std::string Name{ "None" };
	uint32_t Id{};

	friend struct RegisterGameObject;
};

struct RegisterGameObject final
{
	GLAS_MEMBER(GameObject, Name);
	GLAS_MEMBER(GameObject, Id);
	GLAS_MEMBER(GameObject, Transform);
};

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

struct RegisterScene final
{
	GLAS_MEMBER(Scene, Name);
	GLAS_MEMBER(Scene, Objects);
	GLAS_MEMBER(Scene, ObjectsMap);
};

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
	Transform.Rotation.X = distribution(g_Engine);
	Transform.Rotation.Y = distribution(g_Engine);
	Transform.Rotation.Z = distribution(g_Engine);
	Transform.Rotation.W = distribution(g_Engine);
	Transform.Scale.X = distribution(g_Engine);
	Transform.Scale.Y = distribution(g_Engine);
	Transform.Scale.Z = distribution(g_Engine);
	Transform.Translation.X = distribution(g_Engine);
	Transform.Translation.Y = distribution(g_Engine);
	Transform.Translation.Z = distribution(g_Engine);

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

using namespace glas::Storage;

TEST_CASE("Type Storage", "[TypeStorage]")
{
	auto storage = TypeStorage::CopyConstruct<int>(6);
	REQUIRE(*storage.As<int>() == 6);
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
}
