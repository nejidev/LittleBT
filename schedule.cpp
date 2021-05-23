#include "schedule.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common_utils.h"

// #define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

Schedule::Schedule()
{
}

Schedule::~Schedule()
{
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
