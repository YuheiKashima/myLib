#include "ThreadPoolTest.h"

using namespace myLib;

std::atomic<int32_t> TestTask::m_sleeptime = 10;

void ThreadPoolTest::Init() {
	if (m_isInit)
		return;

	ThreadPool::GetInstance().Initalize(4);
	m_input.Ready();

	m_isInit = true;
}

void ThreadPoolTest::Main() {
	if (!m_isInit)
		return;

	while (true) {
		m_input.UpdateState();

		if (m_input.GetPress(VK_ESCAPE))
			break;

		if (m_input.GetTrigger('1')) {
			auto task = std::make_shared<TestTask>();
			m_tasks.emplace_back(task);
			ThreadPool::GetInstance().RegisterTask(task);
		}

		if (m_input.GetTrigger('2')) {
			auto task = std::make_shared<TestTask>();
			m_tasks.emplace_back(task);
			ThreadPool::GetInstance().RegisterTask(task, false);
		}

		if (m_input.GetTrigger('3')) {
			auto task = std::make_shared<TestTask>();
			m_tasks.emplace_back(task);
			ThreadPool::GetInstance().RegisterTask(task, 0, false);
		}

		if (m_input.GetTrigger('4')) {
			auto task = std::make_shared<TestTask>();
			m_tasks.emplace_back(task);
			ThreadPool::GetInstance().RegisterTask(task, 1, false);
		}

		if (m_input.GetTrigger('5')) {
			auto task = std::make_shared<TestTask>();
			m_tasks.emplace_back(task);
			ThreadPool::GetInstance().RegisterTask(task, 2, false);
		}

		if (m_input.GetTrigger('6')) {
			auto task = std::make_shared<TestTask>();
			m_tasks.emplace_back(task);
			ThreadPool::GetInstance().RegisterTask(task, 3, false);
		}

		if (m_input.GetTrigger(VK_SPACE)) {
			ThreadPool::GetInstance().WakeUp();
		}

		if (m_input.GetTrigger('0')) {
			myLib::Logger::Logging(myLib::Logger::ELoggingLevel::LOGLV_INFO, "{}", ThreadPool::GetInstance().GetThreadsState());
		}

		m_input.Ready();

		std::erase_if(m_tasks, [](const std::shared_ptr<TestTask>& task) {
			return task->GetState() == TestState::End;
			});
	}
}

void ThreadPoolTest::Terminate() {
	if (!m_isInit)
		return;
	ThreadPool::GetInstance().WaitForIdle();

	std::erase_if(m_tasks, [](const std::shared_ptr<TestTask>& task) {
		return task->GetState() == TestState::End;
		});

	ThreadPool::GetInstance().Termination();
	m_isInit = false;
}