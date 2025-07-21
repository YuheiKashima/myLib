/**

	@file      Node.h
	@brief
	@details   ~
	@author    Yuuhei Kashima
	@date      30.06.2025
	@copyright © Yuuhei Kashima, 2025. All right reserved.

**/
#ifndef _NODE_
#define _NODE_

#include <future>
#include <tuple>

#include <ThreadPool/ThreadPool.h>
#pragma comment(lib, "ThreadPool.lib")

namespace myLib {
	/**

		@class   Node
		@brief
		@details ~
		@tparam  Args -

	**/
	template<typename... Args>
	class Node :public ThreadPool {
	public:
		Node() {
		}

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

		virtual ~Node() {
		}

		/**
			@brief 引数のFutureを子ノードに登録
			@param  _node -
			@retval       -
		**/
		constexpr size_t Connect(const std::shared_ptr<Node>& _node) {
			if (!_node) return 0;
			m_ChildNodes.emplace_back(_node);
			_node.get()->m_ConnectedCount++;
			return ShrinkNodes();
		}

		/**
			@brief 引数のNodeを切断
			@param  _node -
			@retval       -
		**/
		constexpr size_t Disconnect(const std::shared_ptr<Node>& _node) {
			if (!_node) return 0;
			auto it = find_if(m_ChildNodes.begin(), m_ChildNodes.end(),
				[&](const std::weak_ptr<Node>& child) {
					return child.lock() == _node;
				});
			if (it != m_ChildNodes.end()) {
				it.lock()->m_ConnectedCount--;
				m_ChildNodes.erase(it);
			}
			return ShrinkNodes();
		}

	protected:

		/**
			@brief
		**/
		void RegisterTask(bool _wakeupImmediately = true) override {
			if (m_ReserveArgsFutures.size() == m_ConnectedCount) {
				ThreadPool::RegisterTask();
			}
			else {
				Logger::Logging(Logger::ELoggingLevel::LOGLV_WARN, "Not all connected nodes have registered futures. Current count: {}, Expected count: {}", m_ReserveArgsFutures.size(), m_ConnectedCount.load());
			}
		}

		/**
			@brief
			@param _future -
		**/
		void RegisterFuture(std::future<std::tuple<Args...>> _future) {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			m_ReserveArgsFutures.emplace_back(std::move(_future));
			Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Registered Future. Current count: {} Expected count:{}", m_ReserveArgsFutures.size(), m_ConnectedCount.load());
		}

		/**
			@brief
		**/
		virtual void PreNodeProcess() {
			RegisterTaskConnectedNodes();
		}

		/**
			@brief
		**/
		virtual std::tuple<Args...> NodeExecute() = 0;

		/**
			@brief
		**/
		virtual void PostNodeProcess(std::tuple<Args...> _args) {
			DeregisterInvalidFutures();
		}

		/**
			@brief Futureサイズを取得
			@retval  -
		**/
		constexpr size_t GetArgsCount() const {
			return m_ReserveArgsFutures.size();
		}

		/**
			@brief IndexのFutureを待機し、引数を取得
			@param  _index -
			@retval        -
		**/
		std::tuple<Args...> WaitFutureAndGetArgs(int32_t _index) const {
			if (_index < 0 || _index >= static_cast<int32_t>(m_ReserveArgsFutures.size())) {
				throw std::out_of_range("Index out of range");
			}
			return m_ReserveArgsFutures[_index].get();
		}

		/**
			@brief
			@param _args -
		**/
		void PromiseArgChildNode(std::tuple<Args...> _args) {
			if (m_ChildNodes.empty()) {
				Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "No child nodes connected.");
				return;
			}
			if (m_SendArgsPromises.empty()) {
				Logger::Logging(Logger::ELoggingLevel::LOGLV_WARN, "No promises available to send arguments to child nodes.");
				return;
			}
			for (size_t i = 0; auto& promise : m_SendArgsPromises) {
				if (promise.valid()) {
					promise.set_value(_args);
				}
				else {
					Logger::Logging(Logger::ELoggingLevel::LOGLV_WARN, "Promise at index {} is not valid.", i);
				}
				++i;
			}
		}

	private:
		/**
			@brief 実行
			@param _id -
		**/
		void Execute(std::thread::id _id) override {
			PreNodeProcess();
			PostNodeProcess(NodeExecute());
		}

		/**
			@brief 子ノードをThreadPoolにタスクを登録
		**/
		void RegisterTaskConnectedNodes() {
			ShrinkNodes();
			std::vector<std::promise<std::tuple<Args...>>> promises(m_ChildNodes.size());

			for (size_t i = 0; auto& child : m_ChildNodes) {
				if (auto childPtr = child.lock()) {
					childPtr->m_ReserveArgsFutures.RegisterFuture(promises[i].get_future());
					childPtr->registerTask();
				}
			}
			m_SendArgsPromises = std::move(promises);
		}

		/**
			@brief 無効な子ノードを削除
			@retval  -
		**/
		constexpr size_t ShrinkNodes() {
			erase_if(m_ChildNodes, [&](const std::weak_ptr<Node>& child) {
				return child.expired();
				});

			return m_ChildNodes.size();
		}

		/**
			@brief 無効なFutureを削除
		**/
		void DeregisterInvalidFutures() {
			std::unique_lock<std::mutex> lock(m_FutureMutex);
			std::erase_if(m_ReserveArgsFutures, [&](const std::future<std::tuple<Args...>>& _future) {
				return _future.valid() == false;
				});
		}

		std::atomic<size_t> m_ConnectedCount = 0;

		std::mutex m_FutureMutex;
		std::vector<std::future<std::tuple<Args...>>> m_ReserveArgsFutures;

		std::vector<std::weak_ptr<Node>> m_ChildNodes;
		std::vector<std::promise<std::tuple<Args...>>> m_SendArgsPromises;
	};
}
#endif // _NODE_