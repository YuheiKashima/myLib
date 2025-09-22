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

#include <map>

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
	class Node :public std::enable_shared_from_this<Node<Arg>>, ThreadPool {
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
		size_t ConnectChildNode(const std::shared_ptr<Node<Arg>>& _node) {
			{
				std::lock_guard<std::mutex> lock(m_ChildPromiseMutex);

				// 既に登録されている場合は登録しない
				for (const auto& [wpNode, promise] : m_ChildNodePromiseMap) {
					if (auto spNode = wpNode.lock(); spNode == _node) {
						myLib::Logger::Logger::Logging(myLib::Logger::ELoggingLevel::LOGLV_WARN, std::source_location::current(), "Node::ConnectChildNode() : Node is already registered.");
						return ShlinkExpiredChildNodes();
					}
				}

				myLib::Promise<Arg> promise;
				m_ChildNodePromiseMap.emplace(std::make_pair(std::weak_ptr<Node>(_node), promise));
				_node->ConnectParentNode(shared_from_this(), promise.get_Future());
				myLib::Logger::Logging(myLib::Logger::ELoggingLevel::LOGLV_INFO, std::source_location::current(), "Node::ConnectChildNode() : Node connected.");
			}
			return ShlinkExpiredChildNodes();
		}

		/**
			@brief 子ノード削除
			@param  _node -
			@retval       -
		**/
		size_t DisconnectChildNode(const std::shared_ptr<Node<Arg>>& _node) {
			{
				std::lock_guard<std::mutex> lock(m_ChildPromiseMutex);
				std::erase_if(m_ChildNodePromiseMap, [&_node](const auto& pair) {
					if (auto spNode = pair.first.lock(); spNode == _node) {
						myLib::Logger::Logging(myLib::Logger::ELoggingLevel::LOGLV_INFO, std::source_location::current(), "Node::DisconnectChildNode() : Node disconnected.");
						return true;
					}
					return false;
					});
			}
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
			@brief タスク実行 RegisterTask()で_wakeupImmediatelyをfalseにした場合に使用
		**/
		void Run() {
			ThreadPool::WakeUp();
		}

	protected:

		/**
			@brief 親ノード登録
			@param _future -
		**/
		void ConnectParentNode(std::shared_ptr<Node<Arg>>& _parent, myLib::Future<Arg> _future) {
			std::lock_guard<std::mutex> lock(m_ParentFutureMutex);
			m_ParentNodeFutureMap.emplace(std::make_pair(std::weak_ptr<Node>(_parent), _future));
		}

		/**
			@brief 親ノード登録解除
			@param _future -
		**/
		void DisconnectParentNode(std::shared_ptr<Node<Arg>>& _parent) {
			std::lock_guard<std::mutex> lock(m_ParentFutureMutex);
			std::erase_if(m_ParentNodeFutureMap, [&_parent](const auto& pair) {
				if (auto spNode = pair.first.lock(); spNode == _parent) {
					return true;
				}
				return false;
				});
		}

		/**
			@brief Futureサイズを取得
			@retval  -
		**/
		size_t GetArgsCount() const {
			return m_ParentNodeFutureMap.size();
		}

		/**
			@brief IndexのFutureを待機し、内容を取得
			@param  _index -
			@retval        -
		**/
		Arg WaitFutureAndGetArgs(int32_t _index) {
			std::lock_guard<std::mutex> lock(m_ParentFutureMutex);
			if (_index < 0 || m_ParentNodeFutureMap.size() <= _index)
				throw myLib::MyLibException(myLib::Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Node::WaitFutureAndGetArgs() : Index out of range.");

			auto itr = m_ParentNodeFutureMap.begin();
			std::advance(itr, _index);
			return itr->second.reserve();
		}

		/**
			@brief 事前処理(仮想関数)
		**/
		virtual void PreNodeProcess() {
			RegisterTaskConnectedNodes();
		}

		/**
			@brief メイン処理(純粋仮想関数)
		**/
		virtual Arg NodeExecute() = 0;

		/**
			@brief 事後処理(仮想関数)
		**/
		virtual void PostNodeProcess(Arg _args) {
			PromiseArgChildNode(_args);
		}

		/**
			@brief 子ノードにPromiseを送信
			@param _args -
		**/
		void PromiseArgChildNode(Arg _args) {
			std::lock_guard<std::mutex> lock(m_ChildPromiseMutex);
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
			std::lock_guard<std::mutex> lock(m_ChildPromiseMutex);
			for (auto& [wpNode, promise] : m_ChildNodePromiseMap) {
				if (auto spNode = wpNode.lock(); spNode) {
					spNode->RegisterTask(false);
				}
			}
			Run();
		}

		/**
			@brief 子ノードの弱参照が切れているものを削除
			@retval  - 更新後の子ノード数
		**/
		size_t ShlinkExpiredChildNodes() {
			std::lock_guard<std::mutex> lock(m_ChildPromiseMutex);
			std::erase_if(m_ChildNodePromiseMap, [](const auto& pair) {
				if (pair.first.expired()) {
					return true;
				}
				return false;
				});
		}

		/**
			@brief	親ノードの弱参照が切れているものを削除
			@retval  - 更新後の親ノード数
		**/
		size_t ShilinkExpiredParentNodes() {
			std::lock_guard<std::mutex> lock(m_ParentFutureMutex);
			std::erase_if(m_ParentNodeFutureMap, [](const auto& pair) {
				if (pair.first.expired()) {
					return true;
				}
				return false;
				});
		}

		std::atomic<size_t> m_CntConnectedParentNode{ 0 };

		std::mutex m_ParentFutureMutex;
		std::map<std::weak_ptr<Node>, myLib::Future<Arg>> m_ParentNodeFutureMap;

		std::mutex m_ChildPromiseMutex;
		std::map<std::weak_ptr<Node>, myLib::Promise<Arg>> m_ChildNodePromiseMap;
	};
}
#endif // _NODE_