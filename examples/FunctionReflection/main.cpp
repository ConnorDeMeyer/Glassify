
#include "glassify.h"
#include <iostream>
#include <source_location>

int TestFunction1(int, float)
{
	std::cout << std::source_location::current().function_name();
	return 5;
}

GLAS_FUNCTION(TestFunction1);

int main()
{
	for (auto& test : glas::GetGlobalFunctionsData().FunctionInfoMap)
	{
		std::cout << test.second.Name << '\n';
		if (auto function = test.second.Cast<int, int, float>())
		{
			function(5,5.f);
		}
	}
}