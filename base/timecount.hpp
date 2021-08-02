
#pragma once
#include <chrono>
#include <ctime>
#include <sys/time.h>
#include <string>
using namespace std::chrono;
using std::string;

string getTimenow()
{
	char buf[64] = { 0 };
	static const int kMicroSecondsPerSecond = 1000 * 1000;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	auto microSecondsSinceEpoch_ = tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec;
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	struct tm tm_time;
	gmtime_r(&seconds, &tm_time);
	int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
	snprintf(buf, sizeof(buf), "%4d%02d%02d-%02d:%02d:%02d-%06d",
		tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
		tm_time.tm_hour + 8, tm_time.tm_min, tm_time.tm_sec,
		microseconds);
	return buf;
}

class TimeCount
{
public:
	TimeCount()
	{
		Update();
	}

	void Update()
	{
		_begin = high_resolution_clock::now();
	}

	double getSecond()
	{
		return getMircoSec() * 0.000001;
	}

	long long getMircoSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
	time_t getMillSec()
	{
		return getMircoSec() * 0.001;
	}
	~TimeCount()
	{

	}

private:
	time_point<high_resolution_clock> _begin;
};

