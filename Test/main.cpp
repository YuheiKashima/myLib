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

class TestThread :public ThreadTask {
public:
	void Execute(std::thread::id _workingId) override {
		int32_t delta = m_Delta.load();
		m_Delta += 10; // Increment the delta for the next run
		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Working in thread {} ...waiting {} ms", _workingId, delta);
		std::this_thread::sleep_for(std::chrono::milliseconds(delta));
		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Thread work done threadid:{}, delta was {} ms", _workingId, delta);
	}
private:
	static std::atomic<int32_t> m_Delta;
};
std::atomic<int32_t> TestThread::m_Delta = 10;

class TestPool :public ThreadPool {
public:
	TestPool() {
		alpla = std::make_shared<TestThread>();
		beta = std::make_shared<TestThread>();
		gamma = std::make_shared<TestThread>();
		delta = std::make_shared<TestThread>();
		epsilon = std::make_shared<TestThread>();
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
				RegisterTask(alpla, 0, false);
			}

			if (input.GetTrigger('2')) {
				std::cout << "2 pressed" << std::endl;
				RegisterTask(beta, 1);
			}

			if (input.GetTrigger('3')) {
				std::cout << "3 pressed" << std::endl;
				RegisterTask(gamma, 2);
			}

			if (input.GetTrigger('4')) {
				std::cout << "4 pressed" << std::endl;
				RegisterTask(delta, 3);
			}

			if (input.GetTrigger('5')) {
				std::cout << "5 pressed" << std::endl;
				RegisterTask(epsilon, 4);
			}

			if (input.GetTrigger(VK_SPACE)) {
				WakeUp();
			}

			if (input.GetTrigger('0')) {
				Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "{}", GetThreadsState());
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(33));
		}
	}
	void exit() {
		std::cout << "End Program" << std::endl;
	}

	bool g_exit = false;
	InstantInput input;

	ThreadPool threadPool;
	std::shared_ptr<TestThread> alpla, beta, gamma, delta, epsilon;
};

int main(int argc, char* argv[]) {
	TestPool test;
	test.Run();
	return 0;
}