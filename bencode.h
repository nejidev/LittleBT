#ifndef __BECODE__H_
#define __BECODE__H_

#include <string>
#include <vector>

class Bencode {
private:
	std::string raw;
	std::vector<std::string> announce_list;
	std::string name;
	int creation_time;
	int length;
	int piece_length;

public:
	Bencode();
	Bencode(std::string &str);
	void setRaw(std::string &str);
	void decode();
	~Bencode();
};

#endif // __BECODE__H_
