/**
	@file      inPoolThread.h
	@brief
	@details   ~
	@author    Yuuhei Kashima
	@date      1.06.2025
	@copyright © YuuheiKashima, 2025. All right reserved.
**/
#ifndef _INPOOLTHREAD_
#define _INPOOLTHREAD_

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <Logger.h>
#include <TimeStamp.h>

#include "ThreadTask.h"

#pragma comment(lib, "Logger.lib")
#pragma comment(lib, "TimeStamp.lib")

namespace myLib {
	class ThreadPool;

	/**
		@class   inPoolThread
		@brief
		@details ~
	**/
	class inPoolThread {
	public:
		inPoolThread();
		~inPoolThread();

		void Init(ThreadPool* _pParent);
		void Reset();
		void Terminate();

		void RegisterTask(std::shared_ptr<ThreadTask> _task);
		[[nodiscard]] std::shared_ptr<ThreadTask> Steal();

		void WakeUp();
		void WaitForIdle();
		[[nodiscard]] size_t GetTaskCount() const noexcept;
	private:
		void WaitForInit();
		void WorkFunc();

		std::atomic<bool> m_isTerminated = false;
		std::optional<ThreadPool*> m_pParent = std::nullopt;
		std::thread m_Thread;
		std::deque<std::shared_ptr<ThreadTask>> m_TaskQueue;
		std::mutex m_Mutex;
		std::condition_variable m_ConditionVariable;
	};
}

#endif // _INPOOLTHREAD_