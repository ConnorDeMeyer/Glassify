
#include "glassify.h"
#include <iostream>
#include <source_location>

int TestFunction1(int integer, float floating)
{
	std::cout << std::source_location::current().function_name();
	std::cout << integer << ' ' << floating << '\n';

	return -1;
}

GLAS_FUNCTION(TestFunction1);

int main()
{
	for (auto& test : glas::GetGlobalFunctionsData().FunctionInfoMap)
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


}