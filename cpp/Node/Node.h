#include <Future/Future.h>
#pragma comment(lib, "Future.lib")

namespace myLib {
	/**
		@class   Node
		@brief
		@details ~
		@tparam  Arg -
	**/
	template<typename Arg>
	class Node : public ThreadPool {
	public:
		Node() = default;
		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;
		virtual ~Node() = default;

		constexpr size_t ConnectChildNode(const std::shared_ptr<Node>& _node) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			for (const auto& wpNode : m_ChildNodes) {
				if (auto spNode = wpNode.lock(); spNode == _node) {
					return ShlinkExpiredChildNodes();
				}
			}
			m_ChildNodes.emplace_back(_node);
			return ShlinkExpiredChildNodes();
		}

		constexpr size_t DisconnectChildNode(const std::shared_ptr<Node>& _node) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			// é¿ëïè»ó™
			return ShlinkExpiredChildNodes();
		}

		void RegisterTask(bool _wakeupImmediately = true) override {
			ThreadPool::RegisterTask(_wakeupImmediately);
		}

		void Run() {
			ThreadPool::WakeUp();
		}

	protected:
		void RegisterFuture(myLib::Future<Arg> _future) {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			m_ReserveArgsFutures.emplace_back(_future);
		}

		virtual void PreNodeProcess() {
			RegisterTaskConnectedNodes();
		}

		virtual Arg NodeExecute() = 0;

		virtual void PostNodeProcess(Arg _args) {
			PromiseArgChildNode(_args);
			for (auto& future : m_ReserveArgsFutures) {
				future.reset();
			}
		}

		size_t GetArgsCount() const {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			return m_ReserveArgsFutures.size();
		}

		Arg WaitFutureAndGetArgs(int32_t _index) {
			std::lock_guard<std::mutex> lock(m_FutureMutex);
			if (_index < 0 || static_cast<size_t>(_index) >= m_ReserveArgsFutures.size())
				throw MyLibException(Logger::ELoggingLevel::LOGLV_ERROR, std::source_location::current(), "Node::WaitFutureAndGetArgs() : Index out of range.");
			return m_ReserveArgsFutures[_index].reserve();
		}

		void PromiseArgChildNode(Arg _args) {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			for (auto& promise : m_SendArgsPromises) {
				promise.send(_args);
			}
		}

	private:
		void Execute(std::thread::id _id) override {
			PreNodeProcess();
			PostNodeProcess(NodeExecute());
		}

		void RegisterTaskConnectedNodes() {
			std::lock_guard<std::mutex> lock(m_ChildNodesMutex);
			for (const auto& wpNode : m_ChildNodes) {
				if (auto spNode = wpNode.lock(); spNode) {
					spNode->RegisterTask();
				}
			}
		}

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