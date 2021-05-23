#ifndef __TRACKER__H_
#define __TRACKER__H_

#include <string>
#include <thread>
#include <chrono>

class Tracker {
private:
	bool thread_runing;
	std::thread thread_tid;
	std::string announce;
	std::string infoHash;

private:
	void threadRun();
	std::string buildUrlParam();

public:
	Tracker(std::string announce, std::string infoHash);
	~Tracker();

	void startThread();
	void stopThread();

	void start();
	void stop();
	void complete();
};

#endif // __TRACKER__H_
