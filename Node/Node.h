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
		constexpr size_t ConnectChildNode(const std::shared_ptr<Node>& _node) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);

			// 既に登録されている場合は登録しない
			for (const auto& wpNode : m_ChildNodes) {
				if (auto spNode = wpNode.lock(); spNode == _node) {
					return ShlinkExpiredChildNodes();
				}
			}

			m_ChildNodes.emplace_back(_node);
			return ShlinkExpiredChildNodes();
		}

		/**
			@brief 子ノード削除
			@param  _node -
			@retval       -
		**/
		constexpr size_t DisconnectChildNode(const std::shared_ptr<Node>& _node) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			/*std::erase_if(m_ChildNodes, [&_node](const std::weak_ptr<Node>& wpNode) {
				if (auto spNode = wpNpde.lock(); spNode) {
					return spNode == _node;
				}
				}
			);*/
			return ShlinkExpiredChildNodes();
		}

		/**
			@brief スレッドプールにタスクを登録
			@param _wakeupImmediately - 登録と同時にスレッドを起こすか
		**/
		void RegisterTask(bool _wakeupImmediately = true) override {
		}

		/**
			@brief タスク実行
		**/
		void Run() {
		}

	protected:

		/**
			@brief
			@param _future -
		**/
		void RegisterFuture(std::future<std::tuple<Args...>> _future) {
		}

		/**
			@brief 事前処理
		**/
		virtual void PreNodeProcess() {
		}

		/**
			@brief メイン処理
		**/
		virtual std::tuple<Args...> NodeExecute() = 0;

		/**
			@brief 事後処理
		**/
		virtual void PostNodeProcess(std::tuple<Args...> _args) {
		}

		/**
			@brief Futureサイズを取得
			@retval  -
		**/
		constexpr size_t GetArgsCount() const {
		}

		/**
			@brief IndexのFutureを待機し、内容を取得
			@param  _index -
			@retval        -
		**/
		std::tuple<Args...> WaitFutureAndGetArgs(int32_t _index) {
		}

		/**
			@brief 子ノードにPromiseを送信
			@param _args -
		**/
		void PromiseArgChildNode(std::tuple<Args...> _args) {
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

		std::atomic<size_t> m_CntConnectedParentNode = 0;

		std::mutex m_FutureMutex;
		std::vector<std::future<std::tuple<Args...>>> m_ReserveArgsFutures;

		std::mutex m_ChildNodesMutex;
		std::vector<std::weak_ptr<Node>> m_ChildNodes;
		std::vector<std::promise<std::tuple<Args...>>> m_SendArgsPromises;
	};
}
#endif // _NODE_