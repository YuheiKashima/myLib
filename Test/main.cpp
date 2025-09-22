#include <iostream>
#include <memory>
#include <tuple>

#include "Node\Node.h"
#pragma comment(lib, "Node.lib")

#include "ThreadPool\ThreadPool.h"
#pragma comment(lib, "ThreadPool.lib")

class TestNode : public myLib::Node<std::tuple<int32_t, std::string>> {
public:
	TestNode(int32_t _idx, std::string _comment) : m_Idx(_idx), m_Comment(_comment) {
	}
	~TestNode() override = default;
	std::tuple<int32_t, std::string> NodeExecute() override {
		for (int32_t i = 0; i < this->GetArgsCount(); ++i) {
			auto [idx, comment] = this->WaitFutureAndGetArgs(i);
			std::cout << "Node " << m_Idx << " received from parent Node " << idx << ". Comment: " << comment << std::endl;
		}

		std::cout << "Node " << m_Idx << " executing. Comment: " << m_Comment << std::endl;
		return std::make_tuple(m_Idx, m_Comment);
	}
	int32_t m_Idx = 0;
	std::string m_Comment;
};

int main() {
	myLib::ThreadPool::Initalize(4);

	// Node‚Ì¶¬
	auto parent = std::make_shared<TestNode>(0, "parent");
	auto child1 = std::make_shared<TestNode>(1, "child1");
	auto child2 = std::make_shared<TestNode>(2, "child2");
	auto child3 = std::make_shared<TestNode>(3, "child3");
	// ƒm[ƒh‚ÌÚ‘±

	parent->ConnectChildNode(child1);
	parent->ConnectChildNode(child2);

	child1->ConnectChildNode(child3);
	child2->ConnectChildNode(child3);

	parent->RegisterTask();

	myLib::ThreadPool::WaitForIdle();

	return 0;
}