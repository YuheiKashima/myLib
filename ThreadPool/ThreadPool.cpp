#include "ThreadPool.h"

using namespace myLib;
using namespace std;

vector<unique_ptr<ThreadPool::inPoolThread>>	ThreadPool::ms_Threads;
bool											ThreadPool::ms_isTermination = false;
deque<shared_ptr<ThreadTask>>					ThreadPool::ms_GlobalTaskQ;
mutex											ThreadPool::ms_Mutex;
condition_variable								ThreadPool::ms_CondVariable;
const int32_t									ThreadPool::ms_MinimumRequirements = 3;

ThreadPool::ThreadPool() {
	ms_Threads.resize(thread::hardware_concurrency());

	for (int32_t idx = 0; auto & inThread:ms_Threads) {
		inThread = make_unique<inPoolThread>(idx++, this);
	}
}

ThreadPool::~ThreadPool() {
	WaitForIdle();
	Termination();
}

void ThreadPool::RegisterTask(shared_ptr<ThreadTask> _pTask) {
}

void ThreadPool::WakeUp() {
	for (auto& inThread : ms_Threads) {
		inThread->WakeUp();
	}
}

void ThreadPool::WaitForIdle() {
	{
		unique_lock<mutex> lock(ms_Mutex);
		ms_CondVariable.wait(lock, [this]() {return ms_GlobalTaskQ.empty() || ms_isTermination; });
	}
	for (auto& inThread : ms_Threads) {
		inThread->WaitForIdle();
	}
}

/**
	@brief
**/
void ThreadPool::Termination() {
	{
		unique_lock<mutex> lock(ms_Mutex);
		ms_isTermination = true;
		ms_CondVariable.notify_all();
	}
	for (auto& inThread : ms_Threads) {
		inThread->Termination();
	}
}

/**
	@brief
	@param  _orderThreads -
	@retval               -
**/
int32_t ThreadPool::ResizePoolThreads(int32_t _orderThreads) {
	if (_orderThreads <= 0) {
		_orderThreads = thread::hardware_concurrency();
	}
	if (_orderThreads == ms_Threads.size()) {
		return _orderThreads;
	}
	if (_orderThreads < ms_MinimumRequirements) {
		_orderThreads = ms_MinimumRequirements;
	}

	vector<unique_ptr<inPoolThread>> _move(_orderThreads);

	unique_lock<mutex> lock(ms_Mutex);

	for (int32_t idx = 0; auto & inThread:ms_Threads) {
		_move[idx++] = move(inThread);
	}

	return _orderThreads;
}

//==============================================================================================
ThreadPool::inPoolThread::inPoolThread(int32_t _index, ThreadPool* _pParent) :m_Index(_index), m_pParent(_pParent) {
	m_Thread = thread(&inPoolThread::WorkThreadFunc, this);
}

ThreadPool::inPoolThread::~inPoolThread() {
	WaitForIdle();
	Termination();
	if (m_Thread.joinable())
		m_Thread.join();
}

void ThreadPool::inPoolThread::WakeUp() {
	unique_lock<mutex> lock(m_Mutex);
	m_CondVariable.notify_all();
}

void ThreadPool::inPoolThread::WaitForIdle() {
	unique_lock<mutex> lock(m_Mutex);
	m_CondVariable.wait(lock, [this]() {return m_LocalTaskQ.empty() || m_isTermination; });
}

void ThreadPool::inPoolThread::Termination() {
	unique_lock<mutex> lock(m_Mutex);
	m_isTermination = true;
	m_CondVariable.notify_all();
}

void ThreadPool::inPoolThread::WorkThreadFunc() {
	unique_lock<mutex> lock(m_Mutex, defer_lock);
	shared_ptr<ThreadTask> task;

	while (true) {
		task = nullptr;

		lock.lock();
		if (m_isTermination)break;

		if (task = m_LocalTaskQ.back()) {
			m_LocalTaskQ.pop_back();
		}
		lock.unlock();

		task->Execute();
	}
}