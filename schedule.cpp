#include "schedule.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common_utils.h"

// #define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

Schedule::Schedule()
{
	thread_runing = true;
	thread_tid = std::thread(&Schedule::threadRun, this);
}

Schedule::~Schedule()
{
}

void Schedule::threadRun()
{
	std::unique_lock<std::mutex> locker(mutex);

	while ( thread_runing )
	{
		peer_cond.wait(locker);

		LOG_DEBUG("peer_list len:%ld", peer_list.size());

		for ( auto &peer : peer_list )
		{
			if ( 0 == peer.socket_fd )
			{
				if ( peer.connectTCP() )
				{
					peer.setBencode(&bencode);
					peer.sendHandshake();
				}
				else
				{
					peer.socket_fd = -1;
				}
			}
		}
	}
}

void Schedule::addPeer(PeerEntity peer)
{
	std::lock_guard<std::mutex> locker(mutex);

	for ( auto &item : peer_list )
	{
		if ( item.ip == peer.ip && item.port == peer.port )
		{
			return ;
		}
	}

	peer_list.push_back(peer);
	peer_cond.notify_all();

	LOG_DEBUG("peer_list len:%ld", peer_list.size());
}

void Schedule::startTask(std::string path)
{
	this->path = path;

	int bt_file_size = fileGetSize(path.c_str());

	if ( 0 >= bt_file_size )
	{
		LOG_ERROR("get bt file size failed");
		return ;
	}

	char *bt_file_buffer = (char *)malloc(bt_file_size);
	memset(bt_file_buffer, 0, bt_file_size);

	if ( bt_file_size != fileReadBin(path.c_str(), bt_file_buffer, bt_file_size) )
	{
		LOG_ERROR("get bt file buffer failed");
		return ;
	}

	//bencode parse
	bencode.setRawBuffer(bt_file_buffer, bt_file_size);
	bencode.decode();
	bencode.showFileEntity();

	announce_list = bencode.getAnnounceList();

	//start announce
	for ( auto &announce : announce_list )
	{
		LOG_DEBUG("announce:%s", announce.c_str());

		Tracker *tracker = new Tracker(announce, bencode.getInfoHash());

		tracker->setSchedule(this);
		tracker->startEvent();

		tracker_list.push_back(tracker);
	}

	//wait download

	//hang
	while ( true )
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}

	//cleanup
}
