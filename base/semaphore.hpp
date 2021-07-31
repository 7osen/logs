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
				});//falseʱ���� ��ֹ��ٻ��ѣ��̱߳��ӵȴ�״̬�����ˣ�����ʵ�������������������δ��Ϊtrue��
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