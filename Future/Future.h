/**

	@file      Future.h
	@brief
	@details   ~
	@author    Yuuhei Kashima
	@date      31.08.2025
	@copyright © Yuuhei Kashima, 2025. All right reserved.

**/
#ifndef _FUTURE_
#define _FUTURE_

#include <mutex>
#include <condition_variable>
#include <optional>
#include <tuple>
#include <atomic>

namespace myLib {
	enum class FutureState {
		FutureState_None,
		FutureState_Ready,
		FutureState_Sent,
		FutureState_Reserved
	};

	/**

		@class   Future
		@brief
		@details ~

	**/
	template <typename... Args>

	class Future {
		template <typename... Args>
		class Promise;

	public:
		Future() = delete;
		Future(const Future&) = delete;
		Future(Future&&) = delete;
		Future& operator=(const Future&) = delete;
		Future& operator=(Future&&) = delete;
		virtual ~Future() = default;

		std::tuple<Args...> reserve() {
			std::unique_lock<std::mutex> lock(m_Mutex->lock());
			m_ConditionVariable.wait(lock, [this]() { return m_Args.has_value(); });
		}

		void reset() {
		}

	private:
		Future(std::shared_ptr<std::mutex> _mutex,
			std::shared_ptr<std::condition_variable> _condVariable,
			std::shared_ptr<std::optional<std::tuple<Args...>>> _args,
			std::shared_ptr<std::atomic<size_t>> _version)
			: m_spMutex(_mutex)
			, m_spCondVariable(_condVariable)
			, m_spArgs(_args) {
		}

		std::shared_ptr<std::mutex> m_spMutex;
		std::shared_ptr<std::condition_variable> m_spCondVariable;
		std::shared_ptr<std::optional<std::tuple<Args...>>> m_spArgs;
		std::shared_ptr<FutureState> m_State;
	};
}

#endif