/**

	@file      Promise.h
	@brief
	@details   ~
	@author    Yuuhei Kashima
	@date      31.08.2025
	@copyright © Yuuhei Kashima, 2025. All right reserved.

**/
#pragma once

#ifndef _PROMISE_
#define _PROMISE_

#include <mutex>
#include <condition_variable>
#include <optional>
#include <tuple>
#include <memory>
#include <atomic>
#include "Future.h"

namespace myLib {
	/**

		@class   Promise
		@brief
		@details ~

	**/
	template <typename... Args>
	class Promise {
	public:
		Promise() :
			m_spMutex(std::make_shared<std::mutex>()),
			m_spCondVariable(std::make_shared<std::condition_variable>()),
			m_spArgs(std::make_shared<std::optional<std::tuple<Args...>>>()) {
		}
		Promise(const Promise&) = delete;
		Promise(Promise&&) = delete;
		Promise& operator=(const Promise&) = delete;
		Promise& operator=(Promise&&) = delete;
		virtual ~Promise() = default;

		void operator=(std::tuple< Args...> _args) const {
			set(_args);
		}

		void send(const Args... _args) const {
			{
				std::lock_guard<std::mutex> lock(m_Mutex);
				m_Args = std::make_tuple(_args...);
			}
			m_ConditionVariable.notify_all();
		}

		myLib::Promise<Args...> get_Future() {
			return
		}

	private:
		std::shared_ptr<std::mutex> m_spMutex;
		std::shared_ptr<std::condition_variable> m_spCondVariable;
		std::shared_ptr<std::optional<std::tuple<Args...>>> m_spArgs;
		std::shared_ptr<std::atomic<size_t>> m_spVersion;
	};
}
#endif