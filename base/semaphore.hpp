#pragma once
#include <condition_variable>
#include <mutex>
class semaphore
{
public:
	semaphore()
	{

	}
	~semaphore()
	{

	}

	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			_cv.wait(lock, [this]()->bool {
				return _wakeup > 0;
				});//false时阻塞 防止虚假唤醒（线程被从等待状态唤醒了，但其实共享变量（即条件）并未变为true）
			--_wakeup;
		}
	}

	void wakeup()
	{
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
	}

private:

	std::condition_variable _cv;

	std::mutex _mutex;

	int _wait = 0;

	int _wakeup = 0;
protected:

};