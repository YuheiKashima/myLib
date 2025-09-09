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

#include <vector>
#include <tuple>

#include <ThreadPool/ThreadPool.h>
#pragma comment(lib, "ThreadPool.lib")

#include <Future/Future.h>
#pragma comment(lib, "Future.lib")

namespace myLib {
	/**

		@class   Node
		@brief
		@details ~
		@tparam  Args -

	**/
	template<typename Arg>
	class Node :public ThreadPool {
	public:
		Node() = default;

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

		virtual ~Node() = default;

		/**
			@brief 子ノード登録
			@param  _node -
			@retval       -
		**/
		size_t ConnectChildNode(const std::shared_ptr<Node>& _node) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);

			// 既に登録されている場合は登録しない
			for (const auto& wpNode : m_ChildNodes) {
				if (auto spNode = wpNode.lock(); spNode == _node) {
					return ShlinkExpiredChildNodes();
				}
			}

			m_ChildNodes.emplace_back(_node);
			_node.m_CntConnectedParentNode++;
			return ShlinkExpiredChildNodes();
		}

		/**
			@brief 子ノード削除
			@param  _node -
			@retval       -
		**/
		size_t DisconnectChildNode(const std::shared_ptr<Node>& _node) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			std::erase_if(m_ChildNodes, [&_node](const std::weak_ptr<Node>& wpNode) {
				if (auto spNode = wpNode.lock(); spNode) {
					if (spNode == _node) {
						_node->m_CntConnectedParentNode--;
						return true;
					}
					else {
						return false;
					}
					return false;
				}
				}
			);
			return ShlinkExpiredChildNodes();
		}

		/**
			@brief スレッドプールにタスクを登録
			@param _wakeupImmediately - 登録と同時にスレッドを起こすか
		**/
		void RegisterTask(bool _wakeupImmediately = true) override {
			ThreadPool::RegisterTask(_wakeupImmediately);
		}

		/**
			@brief タスク実行
		**/
		void Run() {
			ThreadPool::WakeUp();
		}

	protected:

		/**
			@brief Future登録
			@param _future -
		**/
		void RegisterFuture(myLib::Future<Arg> _future) {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			m_ReserveArgsFutures.emplace_back(_future);
		}

		/**
			@brief Future削除
			@param _future -
		**/
		void RemoveFuture(myLib::Future<Arg> _future) {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			std::erase_if(m_ReserveArgsFutures, [&_future](const myLib::Future<Arg>& future) {
				return &future == &_future;
				});
		}

		/**
			@brief Futureサイズを取得
			@retval  -
		**/
		size_t GetArgsCount() const {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			return m_ReserveArgsFutures.size();
		}

		/**
			@brief IndexのFutureを待機し、内容を取得
			@param  _index -
			@retval        -
		**/
		Arg WaitFutureAndGetArgs(int32_t _index) {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			if (_index < 0 || static_cast<size_t>(_index) >= m_ReserveArgsFutures.size())
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Node::WaitFutureAndGetArgs() : Index out of range.");
			return m_ReserveArgsFutures[_index].reserve();
		}

		/**
			@brief 事前処理
		**/
		virtual void PreNodeProcess() {
			RegisterTaskConnectedNodes();
		}

		/**
			@brief メイン処理
		**/
		virtual Arg NodeExecute() = 0;

		/**
			@brief 事後処理
		**/
		virtual void PostNodeProcess(Arg _args) {
			m_SendArgsPromises(_args);

			// 登録されているFutureを全てリセット
			for (auto& future : m_ReserveArgsFutures) {
				future.reset();
			}
		}

		/**
			@brief 子ノードにPromiseを送信
			@param _args -
		**/
		void PromiseArgChildNode(Arg _args) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			for (auto& promise : m_SendArgsPromises) {
				promise.send(_args);
			}
		}

	private:
		/**
			@brief スレッドプール継承実行処理
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
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			for (const auto& wpNode : m_ChildNodes) {
				if (auto spNode = wpNode.lock(); spNode) {
					spNode->RegisterTask();
				}
			}
		}

		/**
			@brief 子ノードの弱参照が切れているものを削除
			@retval  - 更新後の子ノード数
		**/
		size_t ShlinkExpiredChildNodes() {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			std::erase_if(m_ChildNodes, [](const std::weak_ptr<Node>& wp) { return wp.expired(); });
			return m_ChildNodes.size();
		}

		std::atomic<size_t> m_CntConnectedParentNode{ 0 };

		std::mutex m_FutureMutex;
		std::vector<myLib::Future<Arg>> m_ReserveArgsFutures;

		std::mutex m_ChildNodesMutex;
		std::vector<std::weak_ptr<Node>> m_ChildNodes;
		std::vector<myLib::Promise<Arg>> m_SendArgsPromises;
	};
}
#endif // _NODE_