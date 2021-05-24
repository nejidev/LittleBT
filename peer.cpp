#include "peer.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>

#include "common_utils.h"

// #define LOG_DEFAULT_LEVEL 2

#include "common_log.h"

PeerEntity::PeerEntity():socket_fd{0},bencode{NULL}
{
}

PeerEntity::PeerEntity(peer_t *peer):PeerEntity()
{
	if ( peer )
	{
		ip = inetNtoaString(peer->ip);
		memcpy(&port, peer->port, sizeof(peer->port));
		port = ntohs(port);
	}
}

PeerEntity::~PeerEntity()
{
}

bool PeerEntity::setBencode(Bencode *bencode)
{
	this->bencode = bencode;

	return true;
}

bool PeerEntity::connectTCP()
{
	struct sockaddr_in addr = {0};
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if ( -1 == fd )
	{
		LOG_ERROR("create socket errno:%d failed", errno);
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(ip.c_str(), &addr.sin_addr);

	if ( -1 == connect(fd, (const struct sockaddr *)&addr, (socklen_t)sizeof(addr)) )
	{
		LOG_ERROR("connect %s:%d errno:%d failed", ip.c_str(), port, errno);
		return false;
	}

	socket_fd = fd;

	return true;
}

bool PeerEntity::closeTCP()
{
	if ( -1 != socket_fd )
	{
		close(socket_fd);
		socket_fd = -1;
	}

	return true;
}

bool PeerEntity::sendHandshake()
{
	if ( -1 == socket_fd || ! bencode )
	{
		return false;
	}

	bt_handshake_t handshake = { 0 };
	std::string peer_id = getLocalPeerId();

	peer_id = urlEncode(peer_id.c_str(), peer_id.length());

	handshake.magic_code = BT_MAGIC_CODE;
	memcpy(handshake.name, BT_PROTOCOL, sizeof(handshake.name));
	memcpy(handshake.sha1, bencode->getInfoHash().c_str(), sizeof(handshake.sha1));
	memcpy(handshake.peer_id, peer_id.c_str(), sizeof(handshake.peer_id));

	if ( sizeof(handshake) != send(socket_fd, (const void *)&handshake, sizeof(handshake), 0) )
	{
		LOG_ERROR("send %s:%d handshake failed errno:%d", ip.c_str(), port, errno);
		return false;
	}

	LOG_DEBUG("handshake scuess");

	return true;
}
