#ifndef __TRACKER__H_
#define __TRACKER__H_

#include <string>
#include <thread>
#include <chrono>

#pragma pack(push)
#pragma pack(1)
typedef struct peer_t
{
	unsigned char ip[4];
	unsigned char port[2];
} peer_t;
#pragma pack(pop)

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

private:
	void threadRun();
	void benDecode(const char *buff, int len);
	void peersDecode(const char *buff, int len);
	std::string buildUrlParam();

public:
	Tracker(std::string announce, std::string infoHash);
	~Tracker();

	void startThread();
	void stopThread();

	void startEvent();
	void stopEvent();
	void completeEvent();
};

#endif // __TRACKER__H_
