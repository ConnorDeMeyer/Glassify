#include "glassify.h"

#include <iostream>
#include <sstream>
#include <random>

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch2/catch.hpp"

using namespace glas::Serialization;

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

GLAS_MEMBER(Transform, Translation);
GLAS_MEMBER(Transform, Rotation);
GLAS_MEMBER(Transform, Scale);

class GameObject final
{
public:
	auto& GetTransform() { return Transform; }
	auto& GetName() { return Name; }
	auto& GetId() { return Id; }
	void Randomize();
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

//int main()
//{
//	GameObject object{};
//
//	object.Randomize();
//
//	glas::Serialization::Serialize(std::cout, object);
//
//	std::stringstream copyStream{};
//	
//	glas::Serialization::Serialize(copyStream, object);
//	GameObject objectCopy{};
//	glas::Serialization::Deserialize(copyStream, objectCopy);
//	
//	std::cout << "\n\n";
//	
//	glas::Serialization::Serialize(std::cout, objectCopy);
//	
//	Scene scene{};
//	scene.GetName() = "Scene01";
//	scene.GetOjects().emplace_back().Randomize();
//	scene.GetOjects().emplace_back().Randomize();
//	scene.GetMap().emplace(0, GameObject{}).first->second.Randomize();
//	scene.GetMap().emplace(1, GameObject{}).first->second.Randomize();
//	scene.GetMap().emplace(2, GameObject{}).first->second.Randomize();
//	
//	std::cout << "\n\n";
//	
//	glas::Serialization::Serialize(std::cout, scene);
//	
//	std::cout << "\n\n";
//	
//	copyStream.str("");
//	glas::Serialization::SerializeBinary(copyStream, scene);
//	Scene sceneCopy{};
//	glas::Serialization::DeserializeBinary(copyStream, sceneCopy);
//	
//	glas::Serialization::Serialize(std::cout, sceneCopy);
//	
//	std::cout << "\n\n";
//	
//	TestClass testClass{};
//	testClass.Array = { 1,2,3,4,5,6 };
//	testClass.Deque = { 1,2,3,4,5,6,7,8 };
//	testClass.ForList = { 1,2,3,4,5,6,7,8 };
//	testClass.List = { 1,2,3,4,5,6,7,8 };
//	testClass.Set = { 1,2,3,4,5,6,7,8 };
//	testClass.UnSet = { 1,2,3,4,5,6,7,8 };
//	testClass.Map = { {1,2}, {3,4}, {5,6} };
//	testClass.UnMap = { {1,2}, {3,4}, {5,6} };
//	glas::Serialization::Serialize(std::cout, testClass);
//	
//	std::cout << "\n\n";
//}

std::random_device g_RandomDevice;
std::default_random_engine g_Engine(g_RandomDevice());

void GameObject::Randomize()
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
}

TEST_CASE("JSon Format", "[JSON]")
{
	SECTION("Vector")
	{
		Vector vector{};
		std::stringstream stream{};

		Serialize(stream, vector);
		std::string streamString = stream.str();

		REQUIRE(streamString == "{\"X\": 0,\"Y\": 0,\"Z\": 0}");

		vector.X = 5.f;
		vector.Y = -1.5f;
		vector.Z = 300.f;

		stream.str("");
		Serialize(stream, vector);
		streamString = stream.str();

		REQUIRE(streamString == "{\"X\": 5,\"Y\": -1.5,\"Z\": 300}");
	}

	SECTION("std::vector")
	{
		{
			std::vector<int> vector{ 1,2,3,4,5,6 };
			std::stringstream stream{};

			Serialize(stream, vector);
			std::string streamString = stream.str();

			REQUIRE(streamString == "[1,2,3,4,5,6]");
		}
		{
			std::vector<int> vector{ };
			std::stringstream stream{};

			Serialize(stream, vector);
			std::string streamString = stream.str();

			REQUIRE(streamString == "[]");
		}
	}

	SECTION("std::map")
	{
		{
			std::map<std::string, std::string> map{{"1", "2"}, {"test space", "-=-=-="}};
			std::stringstream stream{};

			Serialize(stream, map);
			std::string streamString = stream.str();

			REQUIRE(streamString == "{\"1\": \"2\",\"test space\": \"-=-=-=\"}");
		}

		{
			std::map<int, std::string> map{ {1, "2"}, {5, "-=-=-="} };
			std::stringstream stream{};

			Serialize(stream, map);
			std::string streamString = stream.str();

			REQUIRE(streamString == "{1: \"2\",5: \"-=-=-=\"}");
		}
	}
}

TEST_CASE("floating point Serialization", "[float]")
{
	SECTION("float")
	{
		float val{};
		val = INFINITY;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = -INFINITY;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = NAN;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = -NAN;
		REQUIRE_THROWS(Serialize(std::cout, val));
	}
	SECTION("double")
	{
		double val{};
		val = INFINITY;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = -INFINITY;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = NAN;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = -NAN;
		REQUIRE_THROWS(Serialize(std::cout, val));
	}
	SECTION("long double")
	{
		long double val{};
		val = INFINITY;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = -INFINITY;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = NAN;
		REQUIRE_THROWS(Serialize(std::cout, val));
		val = -NAN;
		REQUIRE_THROWS(Serialize(std::cout, val));
	}
}

TEST_CASE("Vector Serialization", "[Vector]")
{
	SECTION("Serialization/Deserialization")
	{
		Vector vector{};
		Vector vector2{};
		std::stringstream stream{};

		Serialize(stream, vector);
		Deserialize(stream, vector2);

		REQUIRE(vector2.X == vector.X);
		REQUIRE(vector2.Y == vector.Y);
		REQUIRE(vector2.Z == vector.Z);

		vector.X = 5.f;
		vector.Y = -1.5f;
		vector.Z = 999.99f;
		
		stream.str("");
		Serialize(stream, vector);
		Deserialize(stream, vector2);
		
		REQUIRE(vector2.X == vector.X);
		REQUIRE(vector2.Y == vector.Y);
		REQUIRE(vector2.Z == vector.Z);
	}

	SECTION("Binary Serialization/Deserialization")
	{
		Vector vector{};
		Vector vector2{};
		std::stringstream stream{};

		SerializeBinary(stream, vector);
		DeserializeBinary(stream, vector2);

		REQUIRE(vector2.X == vector.X);
		REQUIRE(vector2.Y == vector.Y);
		REQUIRE(vector2.Z == vector.Z);

		vector.X = 5.f;
		vector.Y = -1.5f;
		vector.Z = INFINITY;

		stream.str("");
		SerializeBinary(stream, vector);
		DeserializeBinary(stream, vector2);

		REQUIRE(vector2.X == vector.X);
		REQUIRE(vector2.Y == vector.Y);
		REQUIRE(vector2.Z == vector.Z);
	}
}

TEST_CASE("Transform Serialization", "[Transform]")
{
	SECTION("Serialization/Deserialization")
	{
		Transform transform{};
		Transform transform2{};
		std::stringstream stream{};

		Serialize(stream, transform);
		Deserialize(stream, transform2);

		REQUIRE(transform.Translation.X == transform2.Translation.X);
		REQUIRE(transform.Translation.Y == transform2.Translation.Y);
		REQUIRE(transform.Translation.Z == transform2.Translation.Z);
		REQUIRE(transform.Scale.X == transform2.Scale.X);
		REQUIRE(transform.Scale.Y == transform2.Scale.Y);
		REQUIRE(transform.Scale.Z == transform2.Scale.Z);
		REQUIRE(transform.Rotation.X == transform2.Rotation.X);
		REQUIRE(transform.Rotation.Y == transform2.Rotation.Y);
		REQUIRE(transform.Rotation.Z == transform2.Rotation.Z);
		REQUIRE(transform.Rotation.W == transform2.Rotation.W);

		transform.Translation.X = 5.f;
		transform.Translation.Y = -1.5f;
		transform.Translation.Z = 999.99f;
		transform.Scale.X = -500.323f;
		transform.Scale.Y = -0.f;
		transform.Scale.Z = 0;
		transform.Rotation.X = -500.323f;
		transform.Rotation.Y = -0.f;
		transform.Rotation.Z = 23.4214f;
		transform.Rotation.W = 64.513f;

		stream.str("");
		Serialize(stream, transform);
		Deserialize(stream, transform2);

		REQUIRE(transform.Translation.X == transform2.Translation.X);
		REQUIRE(transform.Translation.Y == transform2.Translation.Y);
		REQUIRE(transform.Translation.Z == transform2.Translation.Z);
		REQUIRE(transform.Scale.X == transform2.Scale.X);
		REQUIRE(transform.Scale.Y == transform2.Scale.Y);
		REQUIRE(transform.Scale.Z == transform2.Scale.Z);
		REQUIRE(transform.Rotation.X == transform2.Rotation.X);
		REQUIRE(transform.Rotation.Y == transform2.Rotation.Y);
		REQUIRE(transform.Rotation.Z == transform2.Rotation.Z);
		REQUIRE(transform.Rotation.W == transform2.Rotation.W);
	}

	SECTION("Binary Serialization/Deserialization")
	{
		Transform transform{};
		Transform transform2{};
		std::stringstream stream{};

		SerializeBinary(stream, transform);
		DeserializeBinary(stream, transform2);

		REQUIRE(transform.Translation.X == transform2.Translation.X);
		REQUIRE(transform.Translation.Y == transform2.Translation.Y);
		REQUIRE(transform.Translation.Z == transform2.Translation.Z);
		REQUIRE(transform.Scale.X == transform2.Scale.X);
		REQUIRE(transform.Scale.Y == transform2.Scale.Y);
		REQUIRE(transform.Scale.Z == transform2.Scale.Z);
		REQUIRE(transform.Rotation.X == transform2.Rotation.X);
		REQUIRE(transform.Rotation.Y == transform2.Rotation.Y);
		REQUIRE(transform.Rotation.Z == transform2.Rotation.Z);
		REQUIRE(transform.Rotation.W == transform2.Rotation.W);

		transform.Translation.X = 5.f;
		transform.Translation.Y = -1.5f;
		transform.Translation.Z = 999.99f;
		transform.Scale.X = -500.323f;
		transform.Scale.Y = -0.f;
		transform.Scale.Z = 0;
		transform.Rotation.X = -500.323f;
		transform.Rotation.Y = -0.f;
		transform.Rotation.Z = 23.4214f;
		transform.Rotation.W = 64.513f;

		stream.str("");
		SerializeBinary(stream, transform);
		DeserializeBinary(stream, transform2);

		REQUIRE(transform.Translation.X == transform2.Translation.X);
		REQUIRE(transform.Translation.Y == transform2.Translation.Y);
		REQUIRE(transform.Translation.Z == transform2.Translation.Z);
		REQUIRE(transform.Scale.X == transform2.Scale.X);
		REQUIRE(transform.Scale.Y == transform2.Scale.Y);
		REQUIRE(transform.Scale.Z == transform2.Scale.Z);
		REQUIRE(transform.Rotation.X == transform2.Rotation.X);
		REQUIRE(transform.Rotation.Y == transform2.Rotation.Y);
		REQUIRE(transform.Rotation.Z == transform2.Rotation.Z);
		REQUIRE(transform.Rotation.W == transform2.Rotation.W);
	}
}

TEST_CASE("Tuple Serialization", "[Tuple]")
{
	SECTION("Binary Serialization/Deserialization")
	{
		std::tuple<int, float, double, GameObject> tuple{};

		std::stringstream stream{};
		SerializeBinary(stream, tuple);
		std::tuple<int, float, double, GameObject> tupleCopy{};
		DeserializeBinary(stream, tupleCopy);
	}
}

TEST_CASE("Type Tuple Serialization", "[TypeTuple]")
{
	glas::Storage::TypeTuple tuple(std::tuple<float, GameObject, int*>{ 5.f, GameObject{}, nullptr });
	Serialize(std::cout, tuple);
}
