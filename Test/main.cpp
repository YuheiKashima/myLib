#include <iostream>
#include <chrono>
#include <thread>

#include <ThreadPool/ThreadPool.h>
#pragma comment(lib, "ThreadPool.lib")

#include <InstantInput/InstantInput.h>
#pragma comment(lib, "InstantInput.lib")

#include <Logger/Logger.h>
#pragma comment(lib, "Logger.lib")

#include <Future/Future.h>
#pragma comment(lib, "Future.lib")

#include <thread>
#include <mutex>
#include <condition_variable>

int main() {
	myLib::Logger::Open("Test");
	myLib::InstantInput input; // Initialize InstantInput for keyboard input

	myLib::Promise<std::tuple<int32_t, std::string>> promise;
	auto future = promise.get_Future();

	try {
		{
			std::thread thread1([&promise]() {
				std::this_thread::sleep_for(std::chrono::seconds(3));
				promise.send(std::make_tuple<int32_t, std::string>(1, "Hello"));
				});

			auto [result1, result2] = future.reserve();
			std::cout << result1 << "\n" << result2 << std::endl;
			future.reset();

			thread1.join();
		}
		{
			std::thread thread2([&promise]() {
				std::this_thread::sleep_for(std::chrono::seconds(3));
				promise.send(std::make_tuple<int32_t, std::string>(2, "Hello"));
				});

			auto [result1, result2] = future.reserve();
			std::cout << result1 << "\n" << result2 << std::endl;
			future.reset();

			thread2.join();
		}
	}
	catch (myLib::MyLibException ex) {
		std::cout << ex.what() << std::endl;
	}

	return 0;
}