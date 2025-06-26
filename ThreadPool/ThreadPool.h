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

#include <Logger.h>
#include <TimeStamp.h>

#pragma comment(lib,"Logger.lib")
#pragma comment(lib,"TimeStamp.lib")

namespace myLib {
	/**

		@class   ThreadPool
		@brief
		@details ~
		@tparam

	**/
	class ThreadPool :public std::enable_shared_from_this<ThreadPool> {
	public:
		enum class ThreadState {
			Idle,       //!< スレッドプールがアイドル状態
			Working,    //!< スレッドプールがタスクを処理中
			Terminated  //!< スレッドプールが終了状態
		};

		ThreadPool();
		virtual ~ThreadPool();

		static void Initalize(size_t _orderthreads = ms_DefaultRequirements);
		static std::string GetThreadsState();

		static void WakeUp();
		static void WaitForIdle();
		static void Termination();
	protected:
#ifdef _DEBUG
		void RegisterTask(int32_t d_orderThreadIdx, bool _wakeupImmediately = true);
#endif
		void RegisterTask(bool _wakeupImmediately = true);

		virtual void Execute(std::thread::id _id) = 0;

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

			void RegisterTask(std::shared_ptr<ThreadPool> _task);

			void WakeUp();
			void WaitForIdle();
			void Termination();

			std::optional<std::shared_ptr<ThreadPool>> StealTask();
			std::thread::id GetId() const { return m_Thread.get_id(); }
			const size_t GetQueueSize()const { return m_LocalTaskQ.size(); }
			const ThreadState GetState() const { return m_State; }
		private:
			void WorkFunc();

			bool m_isTermination = true;
			std::thread m_Thread;
			std::deque<std::shared_ptr<ThreadPool>> m_LocalTaskQ;
			std::optional<std::shared_ptr<ThreadPool>> mp_CurrentTask;
			std::mutex m_Mutex;
			std::condition_variable m_CondVariable;
			ThreadState m_State = ThreadState::Idle;
		};

		static size_t GetGrobalQueueSize() { return ms_GlobalTaskQ.size(); }
		static std::optional<std::shared_ptr<ThreadPool>> GetTaskFromGrobalQueue();
		static std::optional<std::shared_ptr<ThreadPool>> StealTaskFromOtherThread();

		static std::atomic<int32_t> ms_InstanceCount;
		static std::map<std::thread::id, std::shared_ptr<inPoolThread>> ms_Threads;
		static bool ms_isTermination;
		static std::deque<std::shared_ptr<ThreadPool>> ms_GlobalTaskQ;
		static std::mutex ms_Mutex;
		static std::condition_variable ms_CondVariable;
		//!< 最低限の動作保障するための必要スレッド数
		static const int32_t ms_MinimumRequirements;
		//!< スレッドプールのデフォルトスレッド数
		static const int32_t ms_DefaultRequirements;
	};
}

#endif // !_THREADPOOL　