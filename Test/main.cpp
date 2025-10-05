#include "ThreadPoolTest.h"

int main() {
	myLib::Logger::Open("TEST");

	ThreadPoolTest test;
	test.Init();
	test.Main();
	test.Terminate();
	return 0;
}