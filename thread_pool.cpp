#include <array>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class MyThreadPool {

public:
	static MyThreadPool& GetInstance() {
		static MyThreadPool instance;
		return instance;
	}
	MyThreadPool(const MyThreadPool& tp) = delete;
	void operator = (const MyThreadPool& tp) = delete;
	MyThreadPool() {
		for (int i = 0; i < num_threads_; ++i) {
			thread_pool_.emplace_back(std::thread(&MyThreadPool::ThreadLoop, this));
		}
	};
	virtual ~MyThreadPool() {
		if (!stopped_) {
			ShutDown();
		}
	}
	void ShutDown() {
		terminate_ = true;
		cv_.notify_all();
		for (std::thread& i : thread_pool_) {
			i.join();
		}
		thread_pool_.clear();
		stopped_ = true;
	}
	template<class F, class... Args>
	auto AddTask(F&& f, Args&&... args) {
		using return_type = typename std::result_of<F(Args...)>::type;

		std::packaged_task<return_type()> task(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<return_type> result = task.get_future();

		{
			std::unique_lock<std::mutex> lock(mu_);
			task_queue_.emplace(std::move(task));
		}
		cv_.notify_one();
		return result.get();
	}
private:
	void ThreadLoop() {
		while (true) {
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(mu_);
				cv_.wait(lock, [this] {return !task_queue_.empty() || terminate_; });
				if (terminate_ && task_queue_.empty()) return;
				task = task_queue_.front();
				task_queue_.pop();
			}
			task();
		}
	}
	const int num_threads_ = std::thread::hardware_concurrency();
	std::vector<std::thread> thread_pool_;
	std::queue<std::function<void()>> task_queue_;
	std::mutex mu_;
	std::condition_variable cv_;
	bool terminate_ = false;
	bool stopped_ = false;
};

int foo(int n) {
	for (int i = 0; i < n; ++i) {
		std::cout << std::this_thread::get_id() << " foo " << n << std::endl;
	}
	return n;
}
int bar() {
	for (int i = 0; i < 10000; ++i) {
		std::cout << std::this_thread::get_id() << " bar" << std::endl;
	}
	return 10000;
}


int main() {
	MyThreadPool tp;
	int res = tp.AddTask(&foo, 100);
	int res2 = tp.AddTask(&bar);
	std::cout << res << ' ' << res2;

}