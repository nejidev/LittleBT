#include "peer.h"

#include <string.h>
#include <arpa/inet.h>

#include "common_utils.h"

PeerEntity::PeerEntity()
{
}

PeerEntity::PeerEntity(peer_t *peer)
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
