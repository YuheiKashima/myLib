#include <iostream>

#include <ThreadPool.h>
#pragma comment(lib, "ThreadPool.lib")

#include <InstantInput.h>
#pragma comment(lib, "InstantInput.lib")

#include <Logger.h>
#pragma comment(lib, "Logger.lib")

#include <thread>
#include <mutex>
#include <condition_variable>

using namespace myLib;

class TestPool :public ThreadPool {
public:
	void Run() {
		RegisterTask();
	}

	void Run(int32_t _orderThreadIdx, bool _wakeupImmidiately = true) {
		RegisterTask(_orderThreadIdx, _wakeupImmidiately);
	}

	void WakeUp() {
		ThreadPool::WakeUp();
	}
private:
	void Execute(std::thread::id _workingId) override {
		int32_t delta = m_Delta.load();
		m_Delta += 10; // Increment the delta for the next run
		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Working in thread {} ...waiting {} ms", _workingId, delta);
		std::this_thread::sleep_for(std::chrono::milliseconds(delta));
		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Thread work done threadid:{}, delta was {} ms", _workingId, delta);
	}

	static std::atomic<int32_t> m_Delta;
};
std::atomic<int32_t> TestPool::m_Delta = 10;

class Test {
public:
	Test() {
		ThreadPool::Initalize(5);
		alpla = std::make_shared<TestPool>();
		beta = std::make_shared<TestPool>();
		gamma = std::make_shared<TestPool>();
		delta = std::make_shared<TestPool>();
		epsilon = std::make_shared<TestPool>();
	}

	void Run() {
		Init();
		mainfunc();
		exit();
	}

private:
	void Init() {
		Logger::Open("Test");
		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Test Program Start {}", "test003");
	}

	void mainfunc() {
		bool loop = true;
		while (loop) {
			input.Ready();
			if (!input.UpdateState()) {
				std::cout << "Input update failed, exiting..." << std::endl;
				g_exit = true;
				loop = false;
			}

			if (GetKeyState(VK_ESCAPE) & 0x80) {
				std::cout << "Escape pressed, exiting..." << std::endl;
				g_exit = true;
				loop = false;
			}

			if (input.GetTrigger('1')) {
				std::cout << "1 pressed" << std::endl;
				alpla->Run(1, false);
			}

			if (input.GetTrigger('2')) {
				std::cout << "2 pressed" << std::endl;
				beta->Run(2, false);
			}

			if (input.GetTrigger('3')) {
				std::cout << "3 pressed" << std::endl;
				gamma->Run(3, false);
			}

			if (input.GetTrigger('4')) {
				std::cout << "4 pressed" << std::endl;
				delta->Run();
			}

			if (input.GetTrigger('5')) {
				std::cout << "5 pressed" << std::endl;
				epsilon->Run();
			}

			if (input.GetTrigger(VK_SPACE)) {
				ThreadPool::WakeUp();
			}

			if (input.GetTrigger('0')) {
				Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "{}", ThreadPool::GetThreadsState());
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(33));
		}
	}
	void exit() {
		std::cout << "End Program" << std::endl;
	}

	bool g_exit = false;
	InstantInput input;

	std::shared_ptr<TestPool> alpla, beta, gamma, delta, epsilon;
};

int main(int argc, char* argv[]) {
	Test test;
	test.Run();
	return 0;
}