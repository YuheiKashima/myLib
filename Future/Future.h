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
#include <memory>

#include <myLibException/myLibException.h>
#pragma comment(lib, "myLibException.lib")

namespace myLib {
	enum class FutureState {
		None,			// 初期状態
		Not_Ready,		// Promise生成直後(Future未生成)
		Send_Ready,		// Future生成後(PromiseがArgsを送信可能)
		Reserve_Ready,	// PromiseがArgsを送信済み(FutureがArgsを受信可能)
		Reserved		// FutureがArgsを受信済み(PromiseがArgsを送信不可、FutureがArgsを受信不可)
	};

	/**

		@class   Future
		@brief
		@details ~

	**/
	template <typename Args>
	class Future {
		template <typename Args>
		friend class Promise;

	public:
		Future() = delete;
		Future(const Future&) = delete;
		Future(Future&&) = delete;
		Future& operator=(const Future&) = delete;
		Future& operator=(Future&&) = delete;
		~Future() {
			std::lock_guard<std::mutex> lock(*m_spMutex);
			m_spArgs->reset();
			*m_spState = FutureState::Not_Ready;
		}

		/**
			@brief Argsを待機し、受信
			@retval  -
		**/
		Args reserve() {
			std::unique_lock<std::mutex> lock(*m_spMutex);
			if (*m_spState == FutureState::Reserved)
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Future::reserve() : Future is already Reserved.");

			m_spCondVariable->wait(lock, [this]() {return m_spArgs->has_value() && *m_spState == FutureState::Reserve_Ready; });

			if (*m_spState != FutureState::Reserve_Ready)
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Future::reserve() : Future state invalid for reserve.");

			Args reservedArgs = m_spArgs->value();
			*m_spState = FutureState::Reserved;
			return reservedArgs;
		}

		/**
			@brief
		**/
		void reset() {
			std::lock_guard<std::mutex> lock(*m_spMutex);
			if (*m_spState != FutureState::Reserved && *m_spState != FutureState::Send_Ready)
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Future::reset() : Future state invalid for reset.");

			m_spArgs->reset();
			*m_spState = FutureState::Send_Ready;
		}

		/**
			@brief
			@retval  -
		**/
		bool expired() const {
			std::lock_guard<std::mutex> lock(*m_spMutex);
			return *m_spState == FutureState::Not_Ready || *m_spState == FutureState::None;
		}

	private:
		Future(std::shared_ptr<std::mutex> _mutex,
			std::shared_ptr<std::condition_variable> _condVariable,
			std::shared_ptr<std::optional<Args>> _args,
			std::shared_ptr<FutureState> _state)
			: m_spMutex(_mutex)
			, m_spCondVariable(_condVariable)
			, m_spArgs(_args)
			, m_spState(_state) {
		}

		std::shared_ptr<std::mutex> m_spMutex;
		std::shared_ptr<std::condition_variable> m_spCondVariable;
		std::shared_ptr<std::optional<Args>> m_spArgs;
		std::shared_ptr<FutureState> m_spState;
	};

	/**

	@class   Promise
	@brief
	@details ~

	**/
	template <typename Args>
	class Promise {
	public:
		Promise() :
			m_spMutex(std::make_shared<std::mutex>()),
			m_spCondVariable(std::make_shared<std::condition_variable>()),
			m_spArgs(std::make_shared<std::optional<Args>>()),
			m_spState(std::make_shared<FutureState>()) {
			*m_spState = FutureState::Not_Ready;
		}
		Promise(const Promise&) = delete;
		Promise(Promise&&) = delete;
		Promise& operator=(const Promise&) = delete;
		Promise& operator=(Promise&&) = delete;
		~Promise() {
			std::lock_guard<std::mutex> lock(*m_spMutex);
			m_spArgs->reset();
			*m_spState = FutureState::None;
		}

		/**
			@brief operator=
			@param _args -
		**/
		void operator=(Args _args) const {
			send(_args);
		}

		/**
			@brief
			@param _args -
		**/
		void send(const Args _args) const {
			switch (*m_spState) {
			case FutureState::Not_Ready:
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Promise is Not generated Future.");
			case FutureState::Send_Ready: {
				std::lock_guard<std::mutex> lock(*m_spMutex);
				*m_spArgs = _args;
				*m_spState = FutureState::Reserve_Ready;
				m_spCondVariable->notify_all();
				return;
			}
			case FutureState::Reserve_Ready:
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Promise is already send.");
			case FutureState::Reserved:
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Future is not call reset.");
			default:
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Future is in an invalid state.");
			}
		}

		/**
			@brief
			@retval  -
		**/
		myLib::Future<Args> get_Future() {
			std::lock_guard<std::mutex> lock(*m_spMutex);

			if (*m_spState != FutureState::Not_Ready)
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Promise::get_Future() : Future is already generated.");

			*m_spState = FutureState::Send_Ready;
			return Future<Args>(m_spMutex, m_spCondVariable, m_spArgs, m_spState);
		}

		/**
			@brief
			@retval  -
		**/
		bool expired() const {
			std::lock_guard<std::mutex> lock(*m_spMutex);
			return *m_spState == FutureState::Not_Ready || *m_spState == FutureState::None;
		}

	private:
		std::shared_ptr<std::mutex> m_spMutex;
		std::shared_ptr<std::condition_variable> m_spCondVariable;
		std::shared_ptr<std::optional<Args>> m_spArgs;
		std::shared_ptr<FutureState> m_spState;
	};
}

#endif