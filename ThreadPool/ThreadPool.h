/**

	@file      ThreadPool.h
	@brief
	@details   ~
	@author    Yuhei kashima
	@date      27.02.2025
	�Q�l�Fhttps://zenn.dev/rita0222/articles/13953a5dfb9698
	�w�j�F���A���^�C���ȃX���b�h���ύX���֎~�i�K���ċN��������j
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

#include "ThreadTask.h"

#pragma comment(lib,"Logger.lib")
#pragma comment(lib,"TimeStamp.lib")

namespace myLib {
	/**

		@class   ThreadPool
		@brief
		@details ~
		@tparam

	**/
	class ThreadPool {
	public:
		enum class ThreadState {
			Idle,       //!< �X���b�h�v�[�����A�C�h�����
			Working,    //!< �X���b�h�v�[�����^�X�N��������
			Terminated  //!< �X���b�h�v�[�����I�����
		};

		ThreadPool(size_t _orderthreads = ms_DefaultRequirements);
		~ThreadPool();

#ifdef _DEBUG
		void RegisterTask(std::shared_ptr<ThreadTask> _task, int32_t d_orderThreadIdx, bool _wakeupImmediately = true);
#endif

		void RegisterTask(std::shared_ptr<ThreadTask> _task, bool _wakeupImmediately = true);
		void WakeUp();
		void WaitForIdle();
		void Termination();

		std::string GetThreadsState() const;
	private:
		/**

			@class   inPoolThread
			@brief
			@details ~

		**/
		class inPoolThread {
		public:
			inPoolThread(ThreadPool* _pParent);
			~inPoolThread();

			void RegisterTask(std::shared_ptr<ThreadTask> _task);

			void WakeUp();
			void WaitForIdle();
			void Termination();

			std::optional<std::shared_ptr<ThreadTask>> StealTask();
			std::thread::id GetId() const { return m_Thread.get_id(); }
			const size_t GetQueueSize()const { return m_LocalTaskQ.size(); }
			const ThreadState GetState() const { return m_State; }
		private:
			void WorkFunc();

			bool m_isTermination = false;
			std::optional<ThreadPool*> mp_Parent;
			std::thread m_Thread;
			std::deque<std::shared_ptr<ThreadTask>> m_LocalTaskQ;
			std::optional<std::shared_ptr<ThreadTask>> mp_CurrentTask;
			std::mutex m_Mutex;
			std::condition_variable m_CondVariable;
			ThreadState m_State = ThreadState::Idle;
		};

		size_t GetGrobalQueueSize() { return ms_GlobalTaskQ.size(); }
		std::optional<std::shared_ptr<ThreadTask>> GetTaskFromGrobalQueue();
		std::optional<std::shared_ptr<ThreadTask>> StealTaskFromOtherThread();

		static std::atomic<int32_t> ms_InstanceCount;
		static std::map<std::thread::id, std::shared_ptr<inPoolThread>> ms_Threads;
		static bool ms_isTermination;
		static std::deque<std::shared_ptr<ThreadTask>> ms_GlobalTaskQ;
		static std::mutex ms_Mutex;
		static std::condition_variable ms_CondVariable;
		//!< �Œ���̓���ۏႷ�邽�߂̕K�v�X���b�h��
		static const int32_t ms_MinimumRequirements;
		//!< �X���b�h�v�[���̃f�t�H���g�X���b�h��
		static const int32_t ms_DefaultRequirements;
	};
}

#endif // !_THREADPOOL�@