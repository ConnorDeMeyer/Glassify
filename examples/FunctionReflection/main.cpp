
#include "glassify.h"
#include <iostream>
#include <source_location>

int GLAS_FUNCTIONTest(int param)
{
	std::cout << "Executed GLAS_FUNCTION Test with parameter " << param << '\n';
	return param * 2;
}

GLAS_FUNCTION(GLAS_FUNCTIONTest);


void GLAS_FUNCTIONOverload()
{
	std::cout << "Overload 1\n";
}

void GLAS_FUNCTIONOverload(int)
{
	std::cout << "Overload 2\n";
}

GLAS_FUNCTION(static_cast<void(*)()>(GLAS_FUNCTIONOverload));
GLAS_FUNCTION(static_cast<void(*)(int)>(GLAS_FUNCTIONOverload));


class Foo
{
public:
	double MethodTest(int parameter)
	{
		std::cout << "Method called\n";
		return static_cast<double>(parameter) / 3.1415;
	}
	void ConstMethodTest() const
	{
		std::cout << "Const method called, var is equal to: " << var << '\n';
	}
private:
	int var = 42;
};

GLAS_MEMBER_FUNCTION(Foo, MethodTest);
GLAS_MEMBER_FUNCTION(Foo, ConstMethodTest);

int main()
{
	{
		auto GLAS_FUNCTIONTestID = GLAS_FUNCTION_ID(GLAS_FUNCTIONTest);

		int result = GLAS_FUNCTIONTest(5);
		std::cout << "GLAS_FUNCTION Test returned " << result << '\n';

		if (const auto GLAS_FUNCTIONTestAddress = GLAS_FUNCTIONTestID.Cast<int, int>())
		{
			result = GLAS_FUNCTIONTestAddress(5);
			std::cout << "GLAS_FUNCTION Test returned " << result << '\n';
		}
	}

	std::cout << "\n\n";

	{
		auto GLAS_FUNCTIONOverload1ID = GLAS_FUNCTION_ID(static_cast<void(*)()>(GLAS_FUNCTIONOverload));
		auto GLAS_FUNCTIONOverload2ID = GLAS_FUNCTION_ID(static_cast<void(*)(int)>(GLAS_FUNCTIONOverload));

		if (const auto GLAS_FUNCTIONOverload1 = GLAS_FUNCTIONOverload1ID.Cast<void>())
		{
			GLAS_FUNCTIONOverload1();
		}
		if (const auto GLAS_FUNCTIONOverload2 = GLAS_FUNCTIONOverload2ID.Cast<void, int>())
		{
			GLAS_FUNCTIONOverload2(0);
		}
	}

	std::cout << "\n\n";

	{
		auto methodID = GLAS_MEMBER_FUNCTION_ID(Foo, MethodTest);
		if (const auto methodAddress = methodID.MethodCast<Foo, double, int>())
		{
			Foo foo;
			auto returnVal = (foo.*methodAddress)(0);
			std::cout << "Method Cast returned " << returnVal << '\n';
		}

		auto constMethodID = GLAS_MEMBER_FUNCTION_ID(Foo, ConstMethodTest);
		if (const auto constMethodAddress = constMethodID.MethodCast<Foo, void>())
		{
			Foo foo;
			(foo.*constMethodAddress)();
		}
	}
}