/**

	@file      ThreadPool.h
	@brief
	@details   ~
	@author    Yuhei kashima
	@date      27.02.2025

**/

#ifndef _THREADPOOL_
#define _THREADPOOL_

#include <thread>
#include <mutex>
#include <optional>
#include <deque>
#include <vector>
#include <memory>
#include <condition_variable>

#include "ThreadTask.h"

namespace myLib {
	/**

		@class   ThreadPool
		@brief
		@details ~
		@tparam

	**/
	class ThreadPool {
	public:
		ThreadPool();
		~ThreadPool();

		void RegisterTask(std::shared_ptr<ThreadTask> _task);
		void WakeUp();
		void WaitForIdle();
		void Termination();

		int32_t ResizePoolThreads(int32_t _orderThreads);

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

			void RegisterTask(std::shared_ptr<ThreadTask> _task);

			void Initialization(int32_t _index, ThreadPool* _pParent);
			void WakeUp();
			void WaitForIdle();
			void Termination();

			std::thread::id GetCurrentThreadId() const { return m_Thread.get_id(); }
		private:
			void WaitForInitialization();
			void WorkThreadFunc();

			bool m_isTermination = false;
			int32_t m_Index = -1;
			std::thread m_Thread;
			std::deque<std::shared_ptr<ThreadTask>> m_LocalTaskQ;
			std::mutex m_Mutex;
			std::condition_variable m_CondVariable;

			std::shared_ptr<ThreadTask> mp_Task = nullptr;

			std::optional<ThreadPool*> mp_Parent;
		};

		int32_t GetCurrentThreadIndex();

		static int32_t ms_InstanceCount;
		static std::vector<std::unique_ptr<inPoolThread>> ms_Threads;
		static bool ms_isTermination;
		static std::deque<std::shared_ptr<ThreadTask>> ms_GlobalTaskQ;
		static std::mutex ms_Mutex;
		static std::condition_variable ms_CondVariable;
		static const int32_t ms_MinimumRequirements;
	};
}

#endif // !_THREADPOOLÅ@