#include "ThreadPool.h"

using namespace myLib;
using namespace std;

int32_t											ThreadPool::ms_InstanceCount = 0;
vector<unique_ptr<ThreadPool::inPoolThread>>	ThreadPool::ms_Threads;
bool											ThreadPool::ms_isTermination = false;
deque<shared_ptr<ThreadTask>>					ThreadPool::ms_GlobalTaskQ;
mutex											ThreadPool::ms_Mutex;
condition_variable								ThreadPool::ms_CondVariable;
const int32_t									ThreadPool::ms_MinimumRequirements = 3;

ThreadPool::ThreadPool() {
	if (ms_InstanceCount++ <= 0) {
		//最初のインスタンスの時だけスレッドプールを作成
		vector<unique_ptr<inPoolThread>> moveThreads(thread::hardware_concurrency());

		for (int32_t idx = 0; auto & inThread:moveThreads) {
			inThread = make_unique<inPoolThread>();
			inThread->Initialization(idx++, this);
		}

		unique_lock<mutex> _lock(ms_Mutex);
		ms_Threads = move(moveThreads);
	}
}

ThreadPool::~ThreadPool() {
	if (--ms_InstanceCount <= 0) {
		//最後のインスタンスの時だけスレッドプールを解放
		WaitForIdle();
		Termination();
	}
}

/**
	@brief
	@param _pTask -
**/
void ThreadPool::RegisterTask(shared_ptr<ThreadTask> _task) {
	//再帰で呼び出されたスレッドのインデックスを取得
	int32_t idx = GetCurrentThreadIndex();
	{
		unique_lock<mutex> lock(ms_Mutex);
		if (idx == -1) {
			//再帰呼び出しでない場合グローバルキューに登録
			ms_GlobalTaskQ.push_back(_task);
		}
		else {
			//再帰呼び出しの場合ローカルキューに登録
			ms_Threads[idx]->RegisterTask(_task);
		}
		WakeUp();
	}
}

/**
	@brief スレッド群のアイドル状態を解除する
**/
void ThreadPool::WakeUp() {
	for (auto& inThread : ms_Threads) {
		inThread->WakeUp();
	}
}

/**
	@brief スレッド群のアイドル状態を待つ
**/
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
	@brief スレッドプール解放
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
	@brief スレッドプールサイズ変更
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

	vector<unique_ptr<inPoolThread>> moveThreads(_orderThreads);
	unique_lock<mutex> lock(ms_Mutex);

	int32_t idx = 0;
	for (auto& inThread : ms_Threads) {
		if (idx >= ms_Threads.size())
			break;
		moveThreads[idx++] = move(inThread);
	}
	if (idx < _orderThreads) {
		for (int32_t i = idx; i < _orderThreads; ++i) {
			moveThreads[i] = make_unique<inPoolThread>(i, this);
		}
	}
	lock.unlock();

	for (auto& inThread : ms_Threads) {
		inThread->WaitForIdle();
		inThread->Termination();
	}

	lock.lock();
	ms_Threads = move(moveThreads);
	return _orderThreads;
}

/**
	@brief 呼び出しスレッドに該当するスレッドプールインデックスを取得
	@retval  -
**/
int32_t ThreadPool::GetCurrentThreadIndex() {
	thread::id threadId = this_thread::get_id();
	for (int32_t idx = 0; auto & inThread:ms_Threads) {
		if (inThread->GetCurrentThreadId() == threadId) {
			return idx;
		}
		++idx;
	}
	return -1;
}

//==============================================================================================
ThreadPool::inPoolThread::inPoolThread() {
	m_Thread = thread(&inPoolThread::WorkThreadFunc, this);
}

ThreadPool::inPoolThread::~inPoolThread() {
	WaitForIdle();
	Termination();
	if (m_Thread.joinable())
		m_Thread.join();
}

/**
	@brief スレッドのローカルキューにタスクを登録する
	@param _task -
**/
void ThreadPool::inPoolThread::RegisterTask(shared_ptr<ThreadTask> _task) {
	unique_lock<mutex> lock(m_Mutex);
	m_LocalTaskQ.push_back(_task);
}

/**
	@brief スレッドの初期化
	@retval  -
**/
void ThreadPool::inPoolThread::Initialization(int32_t _index, ThreadPool* _pParent) {
	unique_lock<mutex> lock(m_Mutex);
	m_Index = _index;
	mp_Parent = _pParent;
	m_CondVariable.notify_all();
}

/**
	@brief スレッドのアイドル状態を解除する
**/
void ThreadPool::inPoolThread::WakeUp() {
	unique_lock<mutex> lock(m_Mutex);
	m_CondVariable.notify_all();
}

/**
	@brief スレッドのアイドル状態を待つ
**/
void ThreadPool::inPoolThread::WaitForIdle() {
	unique_lock<mutex> lock(m_Mutex);
	m_CondVariable.wait(lock, [this]() {return m_LocalTaskQ.empty() || m_isTermination; });
}

/**
	@brief スレッドの終了処理
**/
void ThreadPool::inPoolThread::Termination() {
	unique_lock<mutex> lock(m_Mutex);
	m_isTermination = true;
	m_CondVariable.notify_all();
}

/**
	@brief スレッドの初期化を待つ
**/
void ThreadPool::inPoolThread::WaitForInitialization() {
	unique_lock<mutex> lock(m_Mutex);
	m_CondVariable.wait(lock, [this]() {return mp_Parent.has_value() && m_Index >= 0; });
}

/**
	@brief ワークスレッド
**/
void ThreadPool::inPoolThread::WorkThreadFunc() {
	unique_lock<mutex> lock(m_Mutex, defer_lock);
	WaitForInitialization();

	while (true) {
		mp_Task = nullptr;
		{
			//GetLocalQueue
			lock.lock();
			if (!m_LocalTaskQ.empty()) {
			}
		}
	}
}