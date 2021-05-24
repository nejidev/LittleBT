#ifndef __SCHEDULE__H_
#define __SCHEDULE__H_

#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "peer.h"
#include "bencode.h"
#include "tracker.h"

class Schedule {
private:
	std::string path;
	Bencode bencode;
	std::vector<std::string> announce_list;
	std::vector<Tracker*> tracker_list;
	std::vector<PeerEntity> peer_list;

	bool thread_runing;
	std::thread thread_tid;
	std::mutex mutex;
	std::condition_variable peer_cond;

private:
	void threadRun();

public:
	Schedule();
	~Schedule();

	void addPeer(PeerEntity peer);
	void startTask(std::string path);
};

#endif // __SCHEDULE__H_
