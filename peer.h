#ifndef __PEER__H_
#define __PEER__H_

#include <string>

#include "bencode.h"

#define BT_MAGIC_CODE 0x19
#define BT_PROTOCOL   "BitTorrent protocol"

#pragma pack(push)
#pragma pack(1)
typedef struct peer_t
{
	unsigned char ip[4];
	unsigned char port[2];
} peer_t;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	char magic_code;
	char name[19];
	char extension[8];
	char sha1[20];
	char peer_id[20];
} bt_handshake_t;
#pragma pack(pop)

class PeerEntity
{
public:
	std::string ip;
	int port;
	int socket_fd;

	Bencode *bencode;

public:
	PeerEntity();
	PeerEntity(peer_t *peer);
	~PeerEntity();

	bool setBencode(Bencode *bencode);
	bool connectTCP();
	bool sendHandshake();
	bool closeTCP();
};

#endif // __PEER__H_
