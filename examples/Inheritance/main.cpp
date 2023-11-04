
#include <iostream>
#include "glassify.h"

class Parent1
{
public:
	virtual ~Parent1() = default;
	virtual void Test(){}
	int a{ 1 }, b{ 2 }, c{ 3 };
};

class Parent2
{
public:
	virtual ~Parent2() = default;
	virtual void Test2(){}
	int d{ 4 }, e{ 5 }, f{ 6 };
};

class Child1 : public Parent1, public Parent2
{
public:
	virtual ~Child1() = default;

	int g{ 7 }, h{ 8 }, i{ 9 };
};

GLAS_CHILD(Parent1, Child1)
GLAS_CHILD(Parent2, Child1)

class ComponentBase
{
public:
	virtual ~ComponentBase() = default;
	virtual void Update(float){}
	virtual void Render() const{}
};

class Transform : public ComponentBase
{
public:
	virtual ~Transform() = default;
	void Update(float) override{}
};

template <typename T>
class TestDependency
{
	
};

namespace glas
{
	template <typename T>
	struct AddDependency<TestDependency<T>>
	{
		AddDependency()
		{
			std::cout << "Registered Value with TestDependency\n";
		}
		inline static GlasAutoRegisterTypeOnce<T> RegisterValue{};
	};

	template <>
	struct AddDependency<Transform>
	{
		AddDependency()
		{
			std::cout << "Registered Value with Transform\n";
		}
		inline static GlasAutoRegisterTypeOnce<ComponentBase> RegisterComponent{};
	};
}

GLAS_TYPE(TestDependency<Transform>);

int main()
{
	for (auto& base : glas::TypeId::Create<Child1>().GetInfo().BaseClasses)
	{
		std::cout << base.BaseId.GetInfo().Name;
		std::cout << '\n';
	}

	std::cout << glas::GetClassOffset<Parent1, Child1>();
	std::cout << '\n';
	std::cout << glas::GetClassOffset<Parent2, Child1>();
	std::cout << '\n';

	std::cout << (&ComponentBase::Render == &Transform::Render);
	std::cout << '\n';
	std::cout << (&ComponentBase::Update == &Transform::Update);
	std::cout << '\n';

	for (auto& [id, info] : glas::GetAllTypeInfo())
	{
		std::cout << info.Name << '\n';
	}

	switch (glas::TypeId::Create<int>().GetId())
	{
	case glas::TypeId::Create<int>().GetHash():
		std::cout << "Type int\n";
		break;

	case glas::TypeHash<float>():
		std::cout << "Type float\n";
		break;

	case 0:
		std::cout << "Type was invalid\n";
		break;

	default:
		std::cout << "Type was something else\n";
		break;
	}

	{
		std::unique_ptr<Parent1> child1 = std::make_unique<Child1>();
		std::unique_ptr<Parent2> child2 = std::make_unique<Child1>();

		auto typeId1 = glas::GetTypeIDFromPolymorphic(child1.get());
		std::cout << typeId1.GetInfo().Name << '\n';

		auto typeId2 = glas::GetTypeIDFromPolymorphic(child2.get());
		std::cout << typeId2.GetInfo().Name << '\n';
	}
}