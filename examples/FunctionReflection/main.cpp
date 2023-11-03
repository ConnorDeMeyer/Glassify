
#include "glassify.h"
#include <iostream>
#include <source_location>

int FunctionTest(int param)
{
	std::cout << "Executed Function Test with parameter " << param << '\n';
	return param * 2;
}

GLAS_FUNCTION(FunctionTest);


void FunctionOverload()
{
	std::cout << "Overload 1\n";
}

void FunctionOverload(int)
{
	std::cout << "Overload 2\n";
}

GLAS_FUNCTION(static_cast<void(*)()>(FunctionOverload));
GLAS_FUNCTION(static_cast<void(*)(int)>(FunctionOverload));


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
		auto FunctionTestID = GLAS_FUNCTION_ID(FunctionTest);

		int result = FunctionTest(5);
		std::cout << "Function Test returned " << result << '\n';

		if (const auto FunctionTestAddress = FunctionTestID.Cast<int, int>())
		{
			result = FunctionTestAddress(5);
			std::cout << "Function Test returned " << result << '\n';
		}
	}

	std::cout << "\n\n";

	{
		auto FunctionOverload1ID = GLAS_FUNCTION_ID(static_cast<void(*)()>(FunctionOverload));
		auto FunctionOverload2ID = GLAS_FUNCTION_ID(static_cast<void(*)(int)>(FunctionOverload));

		if (const auto FunctionOverload1 = FunctionOverload1ID.Cast<void>())
		{
			FunctionOverload1();
		}
		if (const auto FunctionOverload2 = FunctionOverload2ID.Cast<void, int>())
		{
			FunctionOverload2(0);
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