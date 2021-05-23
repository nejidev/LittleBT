#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <fstream>

#include "bencode.h"
#include "tracker.h"

#include "common_log.h"

int main(int argc, char **argv)
{
	std::ifstream bt_file_stream;
	char *bt_file_buffer = NULL;
	int bt_file_size = 0;
	Bencode bencode;
	std::vector<std::string> announce_list;
	std::vector<Tracker*> tracker_list;

	if ( 2 != argc )
	{
		LOG_INFO("usage:%s [bt file]", argv[0]);
		return -1;
	}

	bt_file_stream.open(argv[1], std::ios::in | std::ios::binary);

	bt_file_stream.seekg(0, std::ios::end);
	bt_file_size = bt_file_stream.tellg();
	bt_file_stream.seekg(0, std::ios::beg);

	LOG_DEBUG("bt_file_size:%d", bt_file_size);

	bt_file_buffer = (char *)malloc(bt_file_size + 1);
	memset(bt_file_buffer, 0, bt_file_size + 1);

	if ( bt_file_buffer )
	{
		bt_file_stream.read(bt_file_buffer, bt_file_size);
		bt_file_stream.close();

		//LOG_DEBUG("bt_file_stream:%s len:%d", bt_file_buffer, bt_file_size);

		bencode.setRawBuffer(bt_file_buffer, bt_file_size);
		bencode.decode();
		bencode.showFileEntity();

		announce_list = bencode.getAnnounceList();

		for ( auto &announce : announce_list )
		{
			LOG_DEBUG("announce:%s", announce.c_str());

			Tracker *tracker = new Tracker(announce, bencode.getInfoHash());

			tracker->startEvent();

			tracker_list.push_back(tracker);
		}

		//hang
		while ( true )
		{
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}

		free(bt_file_buffer);
		bt_file_buffer = NULL;
	}

	return 0;
}
