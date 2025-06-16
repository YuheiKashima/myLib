#include "ThreadPool.h"

using namespace myLib;
using namespace std;

atomic<int32_t>											ThreadPool::ms_InstanceCount = 0;
map<thread::id, shared_ptr<ThreadPool::inPoolThread>>	ThreadPool::ms_Threads;
bool													ThreadPool::ms_isTermination = false;
deque<shared_ptr<ThreadTask>>							ThreadPool::ms_GlobalTaskQ;
mutex													ThreadPool::ms_Mutex;
condition_variable										ThreadPool::ms_CondVariable;
const int32_t											ThreadPool::ms_MinimumRequirements = 3;
const int32_t 											ThreadPool::ms_DefaultRequirements = 4;

/**
	@brief ThreadPool object constructor
	@details スレッドプールの初期化を行う
**/
ThreadPool::ThreadPool(size_t _orderthreads) {
	if (ms_InstanceCount++ <= 0) {
		if (_orderthreads < ms_MinimumRequirements) {
			_orderthreads = ms_MinimumRequirements;
		}

		for (size_t i = 0; i < _orderthreads; ++i) {
			auto thread = make_shared<inPoolThread>(this);
			ms_Threads.emplace(thread->GetId(), thread);
		}
	}
}

ThreadPool::~ThreadPool() {
	if (--ms_InstanceCount <= 0) {
		//最後のインスタンスの時だけスレッドプールを解放
		WaitForIdle();
		Termination();
	}
}

#ifdef _DEBUG
/**
	@brief タスク登録 ※デバッグ機能
	@param _task            - 登録タスク
	@param d_orderThreadIdx - 登録するスレッドのインデックス（範囲外の場合は通常のタスク登録メソッドへ転送）
	@param _wakeupImmediately - タスク登録後にスレッドを起こすかどうか
**/
void ThreadPool::RegisterTask(shared_ptr<ThreadTask> _task, int32_t d_orderThreadIdx, bool _wakeupImmediately) {
	if (0 <= d_orderThreadIdx && d_orderThreadIdx < ms_Threads.size()) {
		for (int32_t i = 0; auto& inThread:ms_Threads) {
			if (i++ != d_orderThreadIdx) {
				continue;
			}
			Logger::Logging(Logger::ELoggingLevel::LOGLV_DEBUG, "Register task -> [id: {} ]", inThread.first);
			inThread.second->RegisterTask(_task);
		}
		if (_wakeupImmediately) {
			WakeUp();
		}
	}
	else {
		Logger::Logging(Logger::ELoggingLevel::LOGLV_WARN, "Ordered out of range thread index. call normal register function.");
		RegisterTask(_task, _wakeupImmediately);
	}
}
#endif // _DEBUG

/**
	@brief タスク登録
	@param _pTask - class ThreadTaskを継承したクラスのポインタ
	@param _wakeupImmidiately - タスク登録後にスレッドを起こすかどうか(デフォルト：true)
**/
void ThreadPool::RegisterTask(shared_ptr<ThreadTask> _task, bool _wakeupImmidiately) {
	if (ms_isTermination)
		return;

	{
		unique_lock<mutex> lock(ms_Mutex);

		auto itr = ms_Threads.find(this_thread::get_id());
		if (itr != ms_Threads.end()) {
			//再帰呼び出しの場合ローカルキューに登録
			itr->second->RegisterTask(_task);
			Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Recursive registration task [id: {} ] -> [id: {} ]", itr->first, itr->first);
		}
		else {
			//再帰呼び出しでない場合グローバルキューに登録
			ms_GlobalTaskQ.emplace_back(_task);
			Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Registerd task : task -> [Global queue]");
		}
		if (_wakeupImmidiately)
			WakeUp();
	}
}

/**
	@brief スレッド群のアイドル状態を解除する
	@details アイドル状態解除のオーダーを飛ばすだけで、解除条件が整っていなければ再度アイドル状態になる
**/
void ThreadPool::WakeUp() {
	for (auto& inThread : ms_Threads) {
		inThread.second->WakeUp();
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
		inThread.second->WaitForIdle();
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
		inThread.second->Termination();
	}
	Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Terminated thread pool");
}

std::string ThreadPool::GetThreadsState() const {
	std::stringstream state;
	for (const auto& inThread : ms_Threads) {
		state << "Thread ID: " << inThread.first << " State: ";
		switch (inThread.second->GetState()) {
		case ThreadState::Idle:
			state << "Idle";
			break;
		case ThreadState::Working:
			state << "Working";
			break;
		case ThreadState::Terminated:
			state << "Terminated";
			break;
		default:
			state << "Unknown";
			break;
		}
		state << " Queue Size: " << inThread.second->GetQueueSize() << " | ";
		state << endl;
	}
	return state.str();
}

/**
	@brief
	@retval  -
**/
std::optional<std::shared_ptr<ThreadTask>> ThreadPool::GetTaskFromGrobalQueue() {
	unique_lock<mutex> lock(ms_Mutex);

	if (ms_GlobalTaskQ.empty())
		return nullopt;

	shared_ptr<ThreadTask> task = ms_GlobalTaskQ.front();
	ms_GlobalTaskQ.pop_front();

	Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Get task [Global Queue] -> [id: {} ]", this_thread::get_id());

	ms_CondVariable.notify_all();
	return task;
}

/**
	@brief
	@retval  -
**/
std::optional<std::shared_ptr<ThreadTask>> ThreadPool::StealTaskFromOtherThread() {
	const thread::id currentThreadIdx = this_thread::get_id();

	optional<thread::id> idx;
	int32_t max = 0;
	for (auto& inThread : ms_Threads) {
		if (inThread.first == currentThreadIdx)
			continue;

		if (max < inThread.second.get()->GetQueueSize()) {
			idx = inThread.first;
			max = inThread.second.get()->GetQueueSize();
		}
	}

	//待機中タスクが最も多いスレッドからタスクを奪取
	if (idx.has_value()) {
		std::optional<std::shared_ptr<ThreadTask>> stealedTask = ms_Threads[idx.value()]->StealTask();
		if (stealedTask) {
			Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Steal task [id: {0} ] -> [id: {1} ]", idx.value(), currentThreadIdx);
			return stealedTask;
		}
	}

	return nullopt;
}

//==============================================================================================
ThreadPool::inPoolThread::inPoolThread(ThreadPool* _pParent) :mp_Parent(_pParent), m_isTermination(false) {
	m_Thread = thread(&inPoolThread::WorkFunc, this);
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
	m_LocalTaskQ.emplace_back(_task);
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
	m_CondVariable.wait(lock, [this]() {return (m_LocalTaskQ.empty() && !mp_CurrentTask.has_value()) || m_isTermination; });
}

/**
	@brief スレッドの終了処理
	@details スレッド終了フラグをオンにし、スレッド終了を待機する
**/
void ThreadPool::inPoolThread::Termination() {
	unique_lock<mutex> lock(m_Mutex);
	m_isTermination = true;
	m_CondVariable.notify_all();
}

/**
	@brief
	@retval  -
**/
optional<shared_ptr<ThreadTask>> ThreadPool::inPoolThread::StealTask() {
	unique_lock<mutex> lock(m_Mutex);

	if (m_LocalTaskQ.empty())
		return nullopt;

	shared_ptr<ThreadTask> task = m_LocalTaskQ.front();
	m_LocalTaskQ.pop_front();

	m_CondVariable.notify_all();
	return task;
}

/**
	@brief ワークスレッド
**/
void ThreadPool::inPoolThread::WorkFunc() {
	auto GetorStealTask = [&]() -> optional<shared_ptr<ThreadTask>> {
		optional<shared_ptr<ThreadTask>> task = nullopt;
		//ローカルキューが空なら他スレッドの待機中のタスクを奪取
		task = mp_Parent.value()->StealTaskFromOtherThread();
		if (!task.has_value())
			//他スレッドの待機中タスクがない場合グローバルキューからタスクを取得
			task = mp_Parent.value()->GetTaskFromGrobalQueue();

		return task;
		};

	m_State = ThreadState::Working;

	while (true) {
		mp_CurrentTask = nullopt;
		{
			unique_lock<mutex> lock(m_Mutex);
			if (!m_LocalTaskQ.empty()) {
				//ローカルキューからタスクを取得
				mp_CurrentTask = m_LocalTaskQ.front();
				m_LocalTaskQ.pop_front();
			}
		}

		if (!mp_CurrentTask.has_value()) {
			mp_CurrentTask = GetorStealTask();
		}

		//タスクが取得できなければアイドル状態にする
		if (!mp_CurrentTask.has_value()) {
			unique_lock<mutex> lock(m_Mutex);
			m_CondVariable.notify_all();// waitforidle関数でwaitしている処理に通知するためにnotify_allを呼び出す

			m_CondVariable.wait(lock, [&]() {
				// 終了フラグ立っているならwaitを抜ける
				if (m_isTermination) {
					m_State = ThreadState::Terminated;
					return true;
				}

				lock.unlock();
				//タスク取得
				mp_CurrentTask = GetorStealTask();
				lock.lock();

				// タスク取得できればwaitを抜ける、できなければ再度waitする
				if (mp_CurrentTask.has_value()) {
					m_State = ThreadState::Working;
					return true;
				}
				else {
					m_State = ThreadState::Idle;
					//Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Thread {} is waiting for task", this_thread::get_id());
					return false;
				}
				});

			if (m_isTermination) {
				Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Terminated thread [id: {}]", this_thread::get_id());
				break; // 終了フラグが立っている場合はループを抜ける
			}
		}

		if (mp_CurrentTask.has_value())
			mp_CurrentTask.value()->Execute(this_thread::get_id()); // タスクを実行
	}
}