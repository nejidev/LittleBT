#ifndef __FILE_ENTITY__H_
#define __FILE_ENTITY__H_

#include <string>
#include <vector>

class FileEntity {
private:
	std::string path;
	int length;

public:
	FileEntity();
	~FileEntity();
	void setPath(std::string path);
	void setLength(int length);
	std::string getPath() const;
	int getLength() const;
};

#endif // __FILE_ENTITY__H_
