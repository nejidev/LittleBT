#include "file_entity.h"

// #define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

FileEntity::FileEntity():path{NULL}, length{0}
{

}

FileEntity::~FileEntity()
{
}

void FileEntity::setPath(std::string path)
{
	this->path = path;
}

void FileEntity::setLength(int length)
{
	this->length = length;
}

std::string FileEntity::getPath() const
{
	return path;
}

int FileEntity::getLength() const
{
	return length;
}
