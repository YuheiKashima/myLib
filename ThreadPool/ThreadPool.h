/**

	@file      ThreadPool.h
	@brief
	@details   ~
	@author    Yuhei kashima
	@date      27.02.2025
	参考：https://zenn.dev/rita0222/articles/13953a5dfb9698
	指針：リアルタイムなスレッド数変更を禁止（必ず再起動させる）
**/

#ifndef _THREADPOOL_
#define _THREADPOOL_

#include <atomic>
#include <thread>
#include <mutex>
#include <optional>
#include <deque>
#include <vector>
#include <memory>
#include <condition_variable>
#include <map>

#include <Logger/Logger.h>
#include <TimeStamp/TimeStamp.h>

#pragma comment(lib,"Logger.lib")
#pragma comment(lib,"TimeStamp.lib")

namespace myLib {
	/**
		@class   ThreadPoolTask
		@brief
		@details ~スレッドプールに登録するタスクの基底クラス
		@tparam
		**/
	class ThreadPoolTask {
	public:
		ThreadPoolTask() = default;
		virtual ~ThreadPoolTask() = default;

		virtual void Execute(std::thread::id _id) = 0;
	};

	/**

		@class   ThreadPool
		@brief
		@details ~
		@tparam

	**/
	class ThreadPool {
	public:
		enum class ThreadState {
			Idle,       //!< スレッドプールがアイドル状態
			Working,    //!< スレッドプールがタスクを処理中
			Terminated  //!< スレッドプールが終了状態
		};

		static ThreadPool& GetInstance();

		void Initalize(size_t _orderthreads = ms_DefaultRequirements);
		std::string GetThreadsState();

		void WakeUp();
		void WaitForIdle();
		void Termination();

		virtual void RegisterTask(std::shared_ptr<ThreadPoolTask> _task, bool _wakeupImmediately = true);

#ifdef _DEBUG
		virtual void RegisterTask(std::shared_ptr<ThreadPoolTask> _task, int32_t d_orderThreadIdx, bool _wakeupImmediately = true);
#endif
	protected:

	private:
		/**

			@class   inPoolThread
			@brief
			@details ~

		**/
		class inPoolThread {
		public:
			inPoolThread();
			~inPoolThread();

			void RegisterTask(std::shared_ptr<ThreadPoolTask> _task);

			void WakeUp();
			void WaitForIdle();
			void Termination();

			std::optional<std::shared_ptr<ThreadPoolTask>> StealTask();
			std::thread::id GetId() const { return m_Thread.get_id(); }
			const size_t GetQueueSize()const { return m_LocalTaskQ.size(); }
			const ThreadState GetState() const { return m_State; }
		private:
			void WorkFunc();

			bool m_isTermination = true;
			std::thread m_Thread;
			std::deque<std::shared_ptr<ThreadPoolTask>> m_LocalTaskQ;
			std::optional<std::shared_ptr<ThreadPoolTask>> mp_CurrentTask;
			std::mutex m_Mutex;
			std::condition_variable m_CondVariable;
			ThreadState m_State = ThreadState::Idle;
		};

		ThreadPool() = default;
		~ThreadPool() = default;

		//コピーコンストラクタ、ムーブコンストラクタ、コピー代入演算子、ムーブ代入演算子を禁止
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;

		size_t GetGrobalQueueSize() { return m_GlobalTaskQ.size(); }
		std::optional<std::shared_ptr<ThreadPoolTask>> GetTaskFromGrobalQueue();
		std::optional<std::shared_ptr<ThreadPoolTask>> StealTaskFromOtherThread();

		std::map<std::thread::id, std::shared_ptr<inPoolThread>> m_Threads;
		bool m_isTermination = false;
		std::deque<std::shared_ptr<ThreadPoolTask>> m_GlobalTaskQ;
		std::mutex m_Mutex;
		std::condition_variable m_CondVariable;
		//!< 最低限の動作保障するための必要スレッド数
		static const int32_t ms_MinimumRequirements;
		//!< スレッドプールのデフォルトスレッド数
		static const int32_t ms_DefaultRequirements;
	};
}

#endif // !_THREADPOOL　