#ifndef __TRACKER__H_
#define __TRACKER__H_

#include <string>
#include <thread>
#include <chrono>

class Schedule;

class Tracker {
private:
	bool thread_runing;
	std::thread thread_tid;
	std::string announce;
	std::string infoHash;

	int complete;
	int incomplete;
	int downloaded;
	int interval;
	int min_interval;
	int peers;

	Schedule *schedule;

private:
	void threadRun();
	void benDecode(const char *buff, int len);
	void peersDecode(const char *buff, int len);
	std::string buildUrlParam();

public:
	Tracker(std::string announce, std::string infoHash);
	~Tracker();

	void setSchedule(Schedule *schedule);
	void startThread();
	void stopThread();

	void startEvent();
	void stopEvent();
	void completeEvent();
};

#endif // __TRACKER__H_
