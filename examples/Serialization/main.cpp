#include "glassify.h"

#include <iostream>
#include <sstream>
#include <random>

#define CATCH_CONFIG_MAIN
#include "../3rdParty/catch2/catch.hpp"

#include "serialization/Json3rdParty/rapidjson/rapidjson.h"
#include "serialization/Json3rdParty/rapidjson/document.h"
#include "serialization/Json3rdParty/rapidjson/stringbuffer.h"
#include "serialization/Json3rdParty/rapidjson/prettywriter.h"

using namespace glas::Serialization;

namespace SerializeTest
{

#pragma warning(disable:4324) // disable padding warning
	struct alignas(16) Vector
	{
		float X{}, Y{}, Z{};
	};

	GLAS_TYPE(Vector);

	//GlasRegisterMemberType<&::SerializeTest::Vector::X, 4000> RegisterX;
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
		auto& GetTransform() { return GlobalTransform; }
		auto& GetName() { return Name; }
		auto& GetId() { return Id; }
		void Randomize();
	private:
		Transform GlobalTransform{};
		std::string Name{ "None" };
		uint32_t Id{};

		friend struct RegisterGameObject;
	};
}

GLAS_PRIVATE_MEMBER(SerializeTest::GameObject, Name);
GLAS_PRIVATE_MEMBER(SerializeTest::GameObject, Id);
GLAS_PRIVATE_MEMBER(SerializeTest::GameObject, GlobalTransform);

namespace SerializeTest
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
GLAS_PRIVATE_MEMBER(SerializeTest::Scene, Name);
GLAS_PRIVATE_MEMBER(SerializeTest::Scene, Objects);
GLAS_PRIVATE_MEMBER(SerializeTest::Scene, ObjectsMap);

namespace SerializeTest
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

	void GameObject::Randomize()
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
	}

	TEST_CASE("JSon Format", "[JSON]")
	{
		SECTION("Vector")
		{
			Vector vector{};
			std::stringstream stream{};

			SerializeJSon(stream, vector);
			std::string streamString = stream.str();

			REQUIRE(streamString ==
				R"teststr({
    "X": 0.0,
    "Y": 0.0,
    "Z": 0.0
})teststr");

			vector.X = 5.f;
			vector.Y = -1.5f;
			vector.Z = 300.f;

			stream.str("");
			SerializeJSon(stream, vector);
			streamString = stream.str();

			REQUIRE(streamString ==
				R"teststr({
    "X": 5.0,
    "Y": -1.5,
    "Z": 300.0
})teststr");

		}

		SECTION("std::vector")
		{
			{
				std::vector<int> vector{ 1,2,3,4,5,6 };
				std::stringstream stream{};

				SerializeJSon(stream, vector);
				std::string streamString = stream.str();

				REQUIRE(streamString ==
					R"teststr([
    1,
    2,
    3,
    4,
    5,
    6
])teststr");
			}
			{
				std::vector<int> vector{ };
				std::stringstream stream{};

				SerializeJSon(stream, vector);
				std::string streamString = stream.str();

				REQUIRE(streamString == "[]");
			}
		}

		SECTION("std::map")
		{
			{
				std::map<std::string, std::string> map{ {"1", "2"}, {"test space", "-=-=-="} };
				std::stringstream stream{};

				SerializeJSon(stream, map);
				std::string streamString = stream.str();

				REQUIRE(streamString ==
					R"teststr([
    {
        "First": "1",
        "Second": "2"
    },
    {
        "First": "test space",
        "Second": "-=-=-="
    }
])teststr");
			}

			{
				std::map<int, std::string> map{ {1, "2"}, {5, "-=-=-="} };
				std::stringstream stream{};

				SerializeJSon(stream, map);
				std::string streamString = stream.str();

				REQUIRE(streamString ==
					R"teststr([
    {
        "First": 1,
        "Second": "2"
    },
    {
        "First": 5,
        "Second": "-=-=-="
    }
])teststr");
			}
		}
	}



	TEST_CASE("Vector Serialization", "[Vector]")
	{
		SECTION("Serialization/Deserialization")
		{
			Vector vector{};
			Vector vector2{};
			{
				std::stringstream stream{};

				SerializeJSon(stream, vector);
				DeserializeJSon(stream, vector2);

				REQUIRE(vector2.X == vector.X);
				REQUIRE(vector2.Y == vector.Y);
				REQUIRE(vector2.Z == vector.Z);
			}
			{
				std::stringstream stream{};

				vector.X = 5.f;
				vector.Y = -1.5f;
				vector.Z = 999.99f;

				SerializeJSon(stream, vector);
				DeserializeJSon(stream, vector2);

				REQUIRE(vector2.X == vector.X);
				REQUIRE(vector2.Y == vector.Y);
				REQUIRE(vector2.Z == vector.Z);
			}
		}

		SECTION("Binary Serialization/Deserialization")
		{
			Vector vector{};
			Vector vector2{};
			{
				std::stringstream stream{};

				SerializeBinary(stream, vector);
				DeserializeBinary(stream, vector2);

				REQUIRE(vector2.X == vector.X);
				REQUIRE(vector2.Y == vector.Y);
				REQUIRE(vector2.Z == vector.Z);
			}
			{
				std::stringstream stream{};

				vector.X = 5.f;
				vector.Y = -1.5f;
				vector.Z = INFINITY;

				SerializeBinary(stream, vector);
				DeserializeBinary(stream, vector2);

				REQUIRE(vector2.X == vector.X);
				REQUIRE(vector2.Y == vector.Y);
				REQUIRE(vector2.Z == vector.Z);
			}
		}
	}

	TEST_CASE("Transform Serialization", "[Transform]")
	{
		SECTION("Serialization/Deserialization")
		{
			Transform transform{};
			Transform transform2{};
			{
				std::stringstream stream{};

				SerializeJSon(stream, transform);
				DeserializeJSon(stream, transform2);

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
			}

			{
				std::stringstream stream{};

				SerializeJSon(stream, transform);
				DeserializeJSon(stream, transform2);

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

		SECTION("Binary Serialization/Deserialization")
		{
			Transform transform{};
			Transform transform2{};

			{
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
			}

			{
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
			}
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
		SerializeJSon(std::cout, tuple);
	}


	TEST_CASE("Type Vector Serialization", "[TypeVector]")
	{
		{
			glas::Storage::TypeVector vector{ glas::TypeId::Create<Vector>() };

			{
				auto& element = vector.PushBack<Vector>();
				element.X = 1.f;
				element.Y = 2.f;
				element.Z = 3.f;
			}
			{
				auto& element = vector.PushBack<Vector>();
				element.X = 4.f;
				element.Y = 5.f;
				element.Z = 6.f;
			}

			std::stringstream stream;
			SerializeJSon(stream, vector);

			auto result = stream.str();
			REQUIRE(result ==
				R"teststr({
    "Type ID": 14332516166325358969,
    "Data": [
        {
            "X": 1.0,
            "Y": 2.0,
            "Z": 3.0
        },
        {
            "X": 4.0,
            "Y": 5.0,
            "Z": 6.0
        }
    ]
})teststr");
			stream.str(std::move(result));

			glas::Storage::TypeVector vectorCopy;

			DeserializeJSon(stream, vectorCopy);

			REQUIRE(vectorCopy.Size() == 2);
			REQUIRE(vectorCopy.Get<Vector>(0).X == vector.Get<Vector>(0).X);
			REQUIRE(vectorCopy.Get<Vector>(1).Y == vector.Get<Vector>(1).Y);
		}
		{
			glas::Storage::TypeVector vector{ glas::TypeId::Create<Vector>() };

			{
				auto& element = vector.PushBack<Vector>();
				element.X = 1.f;
				element.Y = 2.f;
				element.Z = 3.f;
			}
			{
				auto& element = vector.PushBack<Vector>();
				element.X = 4.f;
				element.Y = 5.f;
				element.Z = 6.f;
			}

			std::stringstream stream;
			glas::Serialization::SerializeBinary(stream, vector);

			glas::Storage::TypeVector vectorCopy;

			glas::Serialization::DeserializeBinary(stream, vectorCopy);

			REQUIRE(vectorCopy.Size() == 2);
			REQUIRE(vectorCopy.Get<Vector>(0).X == vector.Get<Vector>(0).X);
			REQUIRE(vectorCopy.Get<Vector>(1).Y == vector.Get<Vector>(1).Y);
		}
	}

	TEST_CASE("YAML serialization")
	{
		SECTION("Vector")
		{
			Vector vector{};
			std::stringstream stream{};

			SerializeYaml(stream, vector);
			std::string streamString = stream.str();

			REQUIRE(streamString ==
				R"teststr(X: 0
Y: 0
Z: 0)teststr");

			vector.X = 5.f;
			vector.Y = -1.5f;
			vector.Z = 300.f;

			stream.str("");
			SerializeYaml(stream, vector);
			streamString = stream.str();

			REQUIRE(streamString == R"teststr(X: 5
Y: -1.5
Z: 300)teststr");

		}

		SECTION("Transform")
		{
			Transform transform{};
			Transform transform2{};
			{
				std::stringstream stream{};

				SerializeYaml(stream, transform);
				DeserializeYaml(stream, transform2);

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

			{
				std::stringstream stream{};

				std::cout << "\n\n";
				SerializeYaml(std::cout, transform);
				std::cout << "\n\n";

				SerializeYaml(stream, transform);
				DeserializeYaml(stream, transform2);

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

		SECTION("Gameobject")
		{
			std::stringstream stream{};

			GameObject object0{};
			GameObject object1{};

			object0.Randomize();

			std::cout << "\n\n";
			SerializeYaml(std::cout, object0);
			std::cout << "\n\n";

			SerializeYaml(stream, object0);

			DeserializeYaml(stream, object1);

			REQUIRE(object0.GetId() == object1.GetId());
			REQUIRE(object0.GetName() == object1.GetName());
			REQUIRE(object0.GetTransform().Translation.Y == object1.GetTransform().Translation.Y);
		}

		SECTION("Scene")
		{
			std::stringstream stream{};

			Scene scene0{};
			Scene scene1{};

			scene0.GetOjects().emplace_back().Randomize();
			scene0.GetOjects().emplace_back().Randomize();
			scene0.GetOjects().emplace_back().Randomize();

			std::cout << "\n\n";
			SerializeYaml(std::cout, scene0);
			std::cout << "\n\n";

			SerializeYaml(stream, scene0);

			DeserializeYaml(stream, scene1);

			REQUIRE(scene0.GetOjects()[0].GetId() == scene1.GetOjects()[0].GetId());
			REQUIRE(scene0.GetOjects()[1].GetName() == scene1.GetOjects()[1].GetName());
			REQUIRE(scene0.GetOjects()[2].GetTransform().Translation.Y == scene1.GetOjects()[2].GetTransform().Translation.Y);
		}
	}

}