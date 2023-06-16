
#include <iostream>
#include "glassify.h"

class Parent1
{
public:
	virtual void Test(){}
	int a{ 1 }, b{ 2 }, c{ 3 };
};

class Parent2
{
public:
	virtual void Test2(){}
	int d{ 4 }, e{ 5 }, f{ 6 };
};

class Child1 : public Parent1, public Parent2
{
public:
	int g{ 7 }, h{ 8 }, i{ 9 };
};

GLAS_CHILD(Parent1, Child1)
GLAS_CHILD(Parent2, Child1)

class ComponentBase
{
public:
	virtual void Update(float deltaTime);
	virtual void Render() const;
};

class Transform : public ComponentBase
{
public:
	void Update(float deltaTime) override;
};

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

	
}