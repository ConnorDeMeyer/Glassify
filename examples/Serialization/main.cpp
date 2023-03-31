#include "glassify.h"

#include <iostream>
#include <sstream>
#include <random>


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

int main()
{
	GameObject object{};

	object.Randomize();

	glas::Serialization::SerializeType(std::cout, object);

	std::stringstream copyStream{};
	
	glas::Serialization::SerializeType(copyStream, object);
	GameObject objectCopy{};
	glas::Serialization::DeserializeType(copyStream, objectCopy);
	
	std::cout << "\n\n";
	
	glas::Serialization::SerializeType(std::cout, objectCopy);
	
	Scene scene{};
	scene.GetName() = "Scene01";
	scene.GetOjects().emplace_back().Randomize();
	scene.GetOjects().emplace_back().Randomize();
	scene.GetMap().emplace(0, GameObject{}).first->second.Randomize();
	scene.GetMap().emplace(1, GameObject{}).first->second.Randomize();
	scene.GetMap().emplace(2, GameObject{}).first->second.Randomize();
	
	std::cout << "\n\n";
	
	glas::Serialization::SerializeType(std::cout, scene);
	
	std::cout << "\n\n";
	
	copyStream.str("");
	glas::Serialization::SerializeTypeBinary(copyStream, scene);
	Scene sceneCopy{};
	glas::Serialization::DeserializeTypeBinary(copyStream, sceneCopy);
	
	glas::Serialization::SerializeType(std::cout, sceneCopy);
	
	std::cout << "\n\n";
	
	TestClass testClass{};
	testClass.Array = { 1,2,3,4,5,6 };
	testClass.Deque = { 1,2,3,4,5,6,7,8 };
	testClass.ForList = { 1,2,3,4,5,6,7,8 };
	testClass.List = { 1,2,3,4,5,6,7,8 };
	testClass.Set = { 1,2,3,4,5,6,7,8 };
	testClass.UnSet = { 1,2,3,4,5,6,7,8 };
	testClass.Map = { {1,2}, {3,4}, {5,6} };
	testClass.UnMap = { {1,2}, {3,4}, {5,6} };
	glas::Serialization::SerializeType(std::cout, testClass);
	
	std::cout << "\n\n";
}

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