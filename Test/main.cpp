#include <iostream>
#include <memory>

#include "ThreadPool/ThreadPool.h"
#pragma comment(lib,"ThreadPool.lib")

// テスト用のタスククラス
class TestTask : public myLib::ThreadPoolTask {
public:
	void Execute(std::thread::id id) override {
		std::cout << "Task executed on thread: " << id << std::endl;
	}
};

int main() {
	// シングルトンインスタンス取得
	auto& pool = myLib::ThreadPool::GetInstance();

	// スレッドプール初期化
	pool.Initalize(4);

	// タスク登録
	for (int i = 0; i < 8; ++i) {
		pool.RegisterTask(std::make_shared<TestTask>());
	}

	// スレッド状態表示
	std::cout << pool.GetThreadsState() << std::endl;

	// 全タスク完了まで待機
	pool.WaitForIdle();

	// スレッドプール終了
	pool.Termination();

	std::cout << "ThreadPool test finished." << std::endl;
	return 0;
}