#include "inPoolThread.h"

using namespace myLib;
using namespace std;

inPoolThread::inPoolThread() :m_Thread(&inPoolThread::WorkFunc, this) {
}

inPoolThread::~inPoolThread() {
	this->WaitForIdle();
	this->Terminate();
	if (m_Thread.joinable()) {
		m_Thread.join();
	}
}

/**
	@brief
	@param _pParent -
**/
void inPoolThread::Init(ThreadPool* _pParent) {
	{
		unique_lock<mutex> lock(m_Mutex);
		m_pParent = _pParent;
		m_isTerminated = false;
	}
	m_ConditionVariable.notify_all();
}

/**
	@brief
**/
void inPoolThread::Reset() {
	{
		unique_lock<mutex> lock(m_Mutex);
		m_pParent.reset();
	}
	m_ConditionVariable.notify_all();
}

/**
	@brief
**/
void inPoolThread::Terminate() {
	{
		unique_lock<mutex> lock(m_Mutex);
		m_isTerminated = true;
	}
	m_ConditionVariable.notify_all();
}

/**
	@brief
	@param _task -
**/
void inPoolThread::RegisterTask(shared_ptr<ThreadTask> _task) {
	if (m_isTerminated)
		return; // Thread is terminated, do not accept new tasks

	unique_lock<mutex> lock(m_Mutex);
	m_TaskQueue.emplace_back(_task);
}

/**
	@brief
	@retval  -
**/
shared_ptr<ThreadTask> inPoolThread::Steal() {
	unique_lock<mutex> lock(m_Mutex);

	if (m_TaskQueue.empty()) {
		return nullptr; // No tasks available to steal
	}

	shared_ptr<ThreadTask> task = m_TaskQueue.front();
	m_TaskQueue.pop_front();

	return task;
}

/**
	@brief
**/
void inPoolThread::WakeUp() {
}

/**
	@brief
**/
void inPoolThread::WaitForIdle() {
}

/**
	@brief
	@retval  -
**/
size_t inPoolThread::GetTaskCount() const noexcept {
	return m_TaskQueue.size();
}

/**
	@brief
**/
void inPoolThread::WaitForInit() {
}

/**
	@brief
**/
void inPoolThread::WorkFunc() {
}