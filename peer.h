#ifndef __PEER__H_
#define __PEER__H_

#include <string>

#pragma pack(push)
#pragma pack(1)
typedef struct peer_t
{
	unsigned char ip[4];
	unsigned char port[2];
} peer_t;
#pragma pack(pop)

class PeerEntity
{
public:
	std::string ip;
	int port;

public:
	PeerEntity();
	PeerEntity(peer_t *peer);
	~PeerEntity();
};

#endif // __PEER__H_
