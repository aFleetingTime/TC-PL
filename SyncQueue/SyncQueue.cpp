#include <iostream>
#include <future>
#include <atomic>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>

template<typename T>
class SyncQueue
{
public:
	void push(const T &value)
	{
		{
			std::lock_guard lockg(mtx);
			que.push(value);
		}
		cond.notify_one();
	}

	void push(T &&value)
	{
		{
			std::lock_guard lockg(mtx);
			que.push(std::move(value));
		}
		cond.notify_one();
	}

	template<typename Rep, typename Reriod>
	bool push(const T &value, std::chrono::duration<Rep, Reriod> d)
	{
		std::unique_lock ulock(mtx);
		bool ret = cond.wait_for(ulock, d, [this]{ return que.size() < maxSize; });
		if (ret)
		{
			que.push(value);
			cond.notify_one();
		}
		else
			cond.notify_all();
		return ret;
	}

	template<typename Rep, typename Reriod>
	bool push(T &&value, std::chrono::duration<Rep, Reriod> d)
	{
		std::unique_lock ulock(mtx);
		bool ret = cond.wait_for(ulock, d, [this]{ return que.size() < maxSize; });
		if (ret)
		{
			que.push(std::move(value));
			cond.notify_one();
		}
		else
			cond.notify_all();
		return ret;
	}

	void next(T &value)
	{
		std::unique_lock ulock(mtx);
		cond.wait(ulock, [this](){ return !que.empty(); });
		value = que.front();
		que.pop();
	}

	template<typename Rep, typename Reriod>
	bool next(T &value, std::chrono::duration<Rep, Reriod> d)
	{
		std::unique_lock ulock(mtx);
		bool ret = cond.wait_for(ulock, d, [this]{ return !que.empty(); });
		if (ret)
		{
			value = que.front();
			que.pop();
		}
		cond.notify_all();
		return ret;
	}

	bool empty() const
	{
		std::lock_guard lockg(mtx);
		return que.empty();
	}

	static void setMaxSize(std::size_t newSize) { maxSize = newSize; }

private:
	std::queue<T> que;
	std::mutex mtx;
	std::condition_variable cond;
	static inline std::size_t maxSize = 1000;
};

SyncQueue<std::string> sq;
std::mutex outMtx;

void prod(const std::string &name)
{
	std::size_t c = 0;
	while (true)
	{
		if (!sq.push(name.data(), std::chrono::milliseconds(1000)))
			break;
		{
			std::lock_guard lockg(outMtx);
			std::cout << "生产: " << name << '\t' << c << '\n';
		}
		++c;
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::lock_guard lockg(outMtx);
	std::cout << "prod: " << c << std::endl;
}

void cons(std::size_t count)
{
	std::size_t c = 0;
	while (count--)
	{
		std::string s;
		if (!sq.next(s, std::chrono::milliseconds(1000)))
			break;
		{
			std::lock_guard lockg(outMtx);
			std::cout << "消费: " << s << '\t' << c << '\n';
		}
		++c;
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::lock_guard lockg(outMtx);
	std::cout << "cons: " << c << std::endl;
}

constexpr std::launch defaultPolicy = std::launch::async;

int main() try
{
	SyncQueue<std::string>::setMaxSize(10);
#if 0
	auto startTime = std::chrono::steady_clock::now();
	std::thread p(prod, "ABC");
	std::thread p2(prod, "DEF");
	std::thread c(cons, 10000);
	std::thread c2(cons, 10000);
	c2.join();
	c.join();
	p2.join();
	p.join();
	std::cout << (std::chrono::steady_clock::now() - startTime).count() << std::endl;
#endif
	//auto p1 = std::async(defaultPolicy, prod, "ABC");
	//auto c1 = std::async(defaultPolicy, cons, 10000);
	//auto p2 = std::async(defaultPolicy, prod, "DEF");
	//auto c2 = std::async(defaultPolicy, cons, 10000);
	std::future<void> a(std::async(defaultPolicy, prod, "ABC"));
	std::future<void> b(std::async(defaultPolicy, cons, 10000));
	std::future<void> c(std::async(defaultPolicy, prod, "DEF"));
	std::future<void> d(std::async(defaultPolicy, cons, 10000));
	throw 1;		//future析构函数阻塞当前线程，等待async任务执行完毕。
} catch(...) {
	std::cerr << "???????????????????\n";
}
