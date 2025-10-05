#ifndef _THREADPOOLTEST_
#define _THREADPOOLTEST_

#include <iostream>
#include <vector>
#include <format>

#include <ThreadPool/ThreadPool.h>
#pragma comment(lib,"ThreadPool.lib")

#include <InstantInput/InstantInput.h>
#pragma comment(lib,"InstantInput.lib")

#include <Logger/Logger.h>
#pragma comment(lib,"Logger.lib")

enum class TestState {
	Idle,
	Running,
	End
};

class TestTask : public myLib::ThreadPoolTask {
public:
	TestTask() = default;
	~TestTask() override = default;

	void Execute(std::thread::id id) override {
		m_state = TestState::Running;
		myLib::Logger::Logging(myLib::Logger::ELoggingLevel::LOGLV_INFO, "Task started on thread: {}", id);
		std::this_thread::sleep_for(std::chrono::milliseconds(m_sleeptime));
		myLib::Logger::Logging(myLib::Logger::ELoggingLevel::LOGLV_INFO, "Task ended on thread: {} (sleep time: {} ms)", id, m_sleeptime.load());
		m_sleeptime += 10;
		m_state = TestState::End;
	}

	TestState GetState() const {
		return m_state;
	}

private:
	static std::atomic<int32_t> m_sleeptime;
	TestState m_state = TestState::Idle;
};

class ThreadPoolTest {
public:
	void Init();
	void Main();
	void Terminate();
private:
	bool m_isInit = false;
	std::vector<std::shared_ptr<TestTask>> m_tasks;

	myLib::InstantInput m_input;
};

#endif // !_THREADPOOLTEST_