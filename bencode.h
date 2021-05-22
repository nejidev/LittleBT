#ifndef __BECODE__H_
#define __BECODE__H_

#include <string>
#include <vector>

#include "file_entity.h"

class Bencode {
private:
	char *raw_buffer;
	int   raw_buffer_len;
	char info_hash_digest[20];
	bool is_files;

	std::string files_path;
	std::vector<std::string> announce_list;
	std::string name;
	std::string info_hash;
	std::string info_hash_url_encode;
	int info_hash_start;
	int info_hash_end;
	int creation_time;
	int length;
	int piece_length;

	std::vector<FileEntity> file_entity_list;

public:
	Bencode();
	void setRawBuffer(char *buff, int len);
	void decode();
	void showFileEntity();
	void sha1Encode(const char *buff, int len);
	~Bencode();
};

#endif // __BECODE__H_
