
#include "glassify.h"
#include <iostream>
#include <source_location>

int TestFunction1(int integer, float floating)
{
	std::cout << std::source_location::current().function_name();
	std::cout << integer << ' ' << floating << '\n';

	return -1;
}

struct TestStruct
{
	int value{6};
	void Test(int other)
	{
		std::cout << value + other;
	}
};


GLAS_FUNCTION(TestFunction1);
GLAS_MEMBER_FUNCTION(TestStruct, Test);

int main()
{
	for (auto& test : glas::GetGlobalData().FunctionInfoMap)
	{
		std::cout << test.second.Name << '\n';
		if (auto function = test.second.Cast<int, int, float>())
		{
			//function(5,5.f);

			auto parameters = glas::Storage::TypeTuple{ std::tuple<int,float>{5, 2.5f} };
			int result{};
			test.second.Call(parameters, &result);
			std::cout << result;
		}
	}

	std::cout << '\n';

	for (auto& memberFunctions : glas::GetTypeInfo<TestStruct>().MemberFunctions)
	{
		TestStruct struct0{ };
		auto params = glas::Storage::TypeTuple{ std::tuple<TestStruct, int>(struct0, 5) };
		memberFunctions.second.Call(params);
		std::cout << '\n';
	}

}