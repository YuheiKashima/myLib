#include <iostream>

#include <ThreadPool/ThreadPool.h>
#pragma comment(lib, "ThreadPool.lib")

#include <InstantInput/InstantInput.h>
#pragma comment(lib, "InstantInput.lib")

#include <Logger/Logger.h>
#pragma comment(lib, "Logger.lib")

#include <Node/Node.h>
#pragma comment(lib, "Node.lib")

#include <thread>
#include <mutex>
#include <condition_variable>

using namespace myLib;
using namespace std;

//class Test :public Node<int32_t, string> {
//public:
//	Test(int32_t _idx) : m_idx(_idx) {
//		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Test Node Created:{}", m_idx);
//	}
//	~Test() {};
//
//	std::tuple<int32_t, string> NodeExecute() override {
//		for (int32_t i = 0; i < GetArgsCount(); ++i) {
//			auto args = WaitFutureAndGetArgs(i);
//			m_message += "Node:" + to_string(m_idx) + " received message: " + get<1>(args) + "\n";
//			Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Node:{} received message: {}", m_idx, get<1>(args));
//		}
//
//		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Execute Node:{} message:{}", m_idx, m_message);
//		this_thread::sleep_for(chrono::milliseconds(m_sleepTime.load()));
//		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "Node:{} finished execution", m_idx);
//
//		return make_tuple(m_idx, m_message);
//	}
//
//	void run() {
//		RegisterTask();
//	}
//
//private:
//	string m_message;
//	int32_t m_idx = 0;
//	atomic<int32_t> m_sleepTime = 100;
//};

//int main() {
//	//Logger::Open("Test");
//	//ThreadPool::Initalize(4); // Initialize ThreadPool with 4 threads
//	//InstantInput input; // Initialize InstantInput for keyboard input
//	//
//	//array<shared_ptr<Test>, 9> nodes;
//	//for (int i = 0; i < nodes.size(); ++i) {
//	//	nodes[i] = make_shared<Test>(i);
//	//}
//	//
//	//nodes[0]->Connect(nodes[1]);
//	//nodes[0]->Connect(nodes[2]);
//	//
//	//nodes[1]->Connect(nodes[3]);
//	//nodes[1]->Connect(nodes[4]);
//	//nodes[2]->Connect(nodes[4]);
//	//nodes[2]->Connect(nodes[5]);
//	//
//	//nodes[3]->Connect(nodes[6]);
//	//nodes[4]->Connect(nodes[6]);
//	//nodes[4]->Connect(nodes[7]);
//	//nodes[5]->Connect(nodes[7]);
//	//
//	//nodes[6]->Connect(nodes[8]);
//	//nodes[7]->Connect(nodes[8]);
//	//
//	//while (true) {
//	//	input.Ready();
//	//	if (!input.UpdateState()) {
//	//		std::cout << "Input update failed, exiting..." << std::endl;
//	//		break;
//	//	}
//	//
//	//	if (GetKeyState(VK_ESCAPE) & 0x80) {
//	//		std::cout << "Escape pressed, exiting..." << std::endl;
//	//		break;
//	//	}
//	//
//	//	if (input.GetTrigger(VK_F1)) {
//	//		Logger::Logging(Logger::ELoggingLevel::LOGLV_INFO, "F1 key pressed. Starting nodes...");
//	//		nodes.begin()->get()->run(); // Start the first node
//	//	}
//	//}
//	//
//	//ThreadPool::WaitForIdle();
//
//
//
//
//	return 0;
//}

bool g_loop = false;
std::mutex print_mtx_;
std::future<int32_t> g_Future;

template <class T>
void print(const T& x)
{
	std::lock_guard<std::mutex> lk(print_mtx_);
	std::cout << x << std::endl;
}

void process() {
	while (g_loop) {
		if (g_Future.valid()) {
			// 各shared_futureオブジェクトから結果値を取り出す
			g_Future.wait();
			int result = g_Future.get();

			print(result);
		}
	}
}

int main()
{
	InstantInput input;

	std::promise<int32_t> prom;

	g_loop = true;

	std::thread t1(process);
	//std::thread t2(process, f);

	int value = 0; // 何らかの計算
	while (g_loop) {
		input.Ready();
		if (!input.UpdateState()) {
			std::cout << "Input update failed, exiting..." << std::endl;
			break;
		}
		if (GetKeyState(VK_ESCAPE) & 0x80) {
			std::cout << "Escape pressed, exiting..." << std::endl;
			g_loop = false;
			break;
		}
		if (input.GetTrigger(VK_F1)) {
			std::cout << "F1 key pressed. Setting value..." << std::endl;
			prom.set_value(value++);  // 計算結果を設定する
		}
		if (input.GetTrigger(VK_F2)) {
			if (!g_Future.valid()) {
				std::cout << "F2 key pressed. Setting future..." << std::endl;
				g_Future = prom.get_future();
			}
		}
	}

	t1.join();
	//t2.join();

	return 0;
}