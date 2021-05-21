#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <fstream>

#include "bencode.h"
#include "common_log.h"

int main(int argc, char **argv)
{
	std::string bt_file_content = "";
	std::ifstream bt_file_stream;
	char *bt_file_buffer = NULL;
	int bt_file_size = 0;
	Bencode bencode; 

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

	bt_file_size++;
	bt_file_buffer = (char *)malloc(bt_file_size);
	memset(bt_file_buffer, 0, bt_file_size);

	bt_file_stream.read(bt_file_buffer, bt_file_size);
	bt_file_stream.close();

	bt_file_content = bt_file_buffer;

	if ( bt_file_buffer )
	{
		free(bt_file_buffer);
		bt_file_buffer = NULL;
	}

	//LOG_DEBUG("bt_file_stream:%s", bt_file_content.c_str());

	bencode.setRaw(bt_file_content); 
	bencode.decode(); 

	return 0;
}
