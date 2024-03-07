#include "Socket.hpp"
#include <unistd.h> // close
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h> // strcpy
#include <chrono>
#include <array>

Socket::Socket( int type ) : _handle(-1), _type(type), _ping_us(0), _ping_ms(0), _sent(0), _lost(0)
{

}

Socket::~Socket( void )
{
	if (_handle != -1)
		close(_handle);
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

bool Socket::Open( unsigned short port )
{
	// UDP socket
	_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (_handle <= 0) {
        std::cerr << "failed to create socket" << std::endl;
        return (false);
    }

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = (_type == SOCKET::SERVER) ? htons(port) : 0;

	if (bind(_handle, (const sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
		std::cerr << "failed to bind socket" << std::endl;
		return (false);
	}

	int nonBlocking = 1;
    if (fcntl(_handle, F_SETFL, O_NONBLOCK, nonBlocking ) == -1) {
		std::cerr << "failed to set non-blocking" << std::endl;
        return (false);
	}
	return (true);
}

void Socket::Close( void )
{
	close(_handle);
	_handle = -1;
}

bool Socket::IsOpen( void ) const
{
	return (_handle != -1);
}

bool Socket::Send( const Address & destination, const void * data, int size )
{
	if (size > PACKET_SIZE_LIMIT) {
		std::cerr << "packet too big to be sent (" << size << " vs " << PACKET_SIZE_LIMIT << ')' << std::endl;
		return (false);
	}
	sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(destination.getAddress());
    addr.sin_port = htons(destination.getPort());

	t_client *dst = NULL;
	int index = 0;
	if (_type == SOCKET::SERVER) {
		for (auto &c : _clients) {
			if (c.ip == destination) {
				dst = &c;
				break ;
			}
			++index;
		}
	} else if (_type == SOCKET::CLIENT) {
		if (_clients.empty()) {
			_clients.push_back({destination});
		}
		dst = &_clients[0];
	}

	if (!dst) {
		std::cerr << "failed to send packet, client not found" << std::endl;
		return (false);
	}

	if (++dst->timeout > 5 * 20) { // we send packets at a rate of 20/seconds
		// disconnection
		if (_type == SOCKET::SERVER) {
			std::cout << "disconnection from " << dst->ip << ", id is " << dst->id << std::endl;
		} else {
			std::cout << "disconnected from server" << std::endl;
		}
		_clients.erase(_clients.begin() + index);
		return (false);
	}

	t_packet packet = {PROTOCOL_ID, dst->sequence, dst->ack, dst->bitfield, {}};
	memmove(packet.data, data, size);
	size += 12;
	// std::cout << "sending packet " << packet.protocol_id << " - " << packet.sequence << " - " << packet.ack << " - " << packet.ack_bitfield << ": " << packet.data << std::endl;

	// int sent_bytes;
	// if (0 && _type == SOCKET::CLIENT && dst->sequence & 1) { // simulate packet loss
	// 	sent_bytes = size;
	// } else if (_type == SOCKET::CLIENT) { // simulate wrong order of sequence
	// 	packet.sequence ^= 0x1;
	// sent_bytes = sendto(_handle, static_cast<const void*>(&packet.protocol_id), size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	// } else {
	int sent_bytes = sendto(_handle, static_cast<const void*>(&packet.protocol_id), size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	// }

    if (sent_bytes != size) {
        std::cerr << "failed to send packet" << std::endl;
        return (false);
    }
	// std::cout << "sent packet of size " << sent_bytes << " to " << destination << std::endl;
	if (_type == SOCKET::CLIENT) {
		// update pending packets
		auto timestamp = std::chrono::high_resolution_clock::now();
		int64_t ms = std::chrono::time_point_cast<std::chrono::microseconds>(timestamp).time_since_epoch().count();
		_pending_packets.push_back({dst->sequence, ms});
		++_sent;
		if (_pending_packets.size() > 32) {
			_pending_packets.pop_front();
			++_lost;
		}
	}
	++dst->sequence;
	return (true);
}

bool Socket::Send( const Address & destination, std::string str )
{
	return (Send(destination, str.c_str(), str.size()));
}

void Socket::Broadcast( const void * data, int size )
{
	if (_type == SOCKET::CLIENT) return ; // only server is allowed to broadcast

	for (auto &c : _clients) {
		// TODO check deconnection here
		Send(c.ip, data, size);
	}
}

void Socket::Broadcast( std::string str )
{
	Broadcast(str.c_str(), str.size());
}

static inline bool sequence_greater_than( uint16_t s1, uint16_t s2 )
{
	return (s1 > s2 && s1 - s2 <= UINT16_HALF) || (s1 < s2 && s2 - s1 > UINT16_HALF);
}

static inline int diff_wrap_around( uint16_t s1, uint16_t s2 )
{
	if (s1 > s2 && s1 - s2 <= UINT16_HALF) {
		return (s1 - s2);
	} else if (s1 < s2 && s2 - s1 > UINT16_HALF) { // wrap_around
		return (s1 + 0x10000 - s2);
	}
	return (-1); // s2 > s1
}

int Socket::Receive( Address & sender, void * data, int size )
{
	sockaddr_in from;
	socklen_t fromLength = sizeof(from);

	t_packet packet;
	int bytes = recvfrom(_handle, static_cast<void *>(&packet), 12 + size, 0, (sockaddr*)&from, &fromLength);

	// discard wrong recv

	bytes -= 12;
	if (bytes <= 0)
	    return (bytes);
	if (bytes >= PACKET_SIZE_LIMIT) {
		std::cerr << "packet discarded because too big" << std::endl;
		return (0);
	}
	if (packet.protocol_id != PROTOCOL_ID) {
		std::cerr << "packet discarded becausee wrong protocol id used: " << packet.protocol_id << std::endl;
		return (0);
	}
	sender = Address(ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));
	t_client *src = NULL;
	for (auto &c : _clients) {
		if (c.ip == sender) {
			src = &c;
			break ;
		}
	}
	if (!src) {
		if (_type == SOCKET::CLIENT) {
			std::cerr << "packet discarded because source is not server" << std::endl;
			return (0);
		}
		if (_clients.size() == CLIENT_SIZE_LIMIT) {
			std::cerr << "server max capacity reached, can't accept new connection" << std::endl;
			return (0);
		}
		std::array<bool, CLIENT_SIZE_LIMIT> occupied = {false};
		occupied[0] = true; // no one can be client 0
		for (auto &c : _clients) {
			occupied[c.id] = true;
		}
		_clients.push_back({sender});
		src = &_clients.back();
		for (int i = 0; i < CLIENT_SIZE_LIMIT; ++i) {
			if (!occupied[i]) {
				src->id = i;
				break ;
			}
		}
		std::cout << "new connection from " << src->ip << ", id is " << src->id << std::endl;
	}
	src->timeout = 0; // reset timeout
	if (!sequence_greater_than(packet.sequence, src->ack)) {
		std::cerr << "packet discarded because sequence smaller than remote" << std::endl;
		if (_type == SOCKET::SERVER) { // update ack_bitfield
			int diff = diff_wrap_around(src->ack, packet.sequence);
			src->bitfield |= (1 << (diff - 1));
		}
		return (0);
	}

	if (_type == SOCKET::CLIENT) {
		// update own packet loss
		for (auto &pack : _pending_packets) {
			if (pack.first == packet.ack) {
				auto endstamp = std::chrono::high_resolution_clock::now();
				int64_t end = std::chrono::time_point_cast<std::chrono::microseconds>(endstamp).time_since_epoch().count();
				int ping = end - pack.second;
				_ping_us += 0.1 * (ping - _ping_us); // exponentially smoothed moving average
				_ping_ms = _ping_us * 0.001f;
				// std::cout << "received packet " << packet.ack << " | " << pack.second << " -> " << end << ", packet's ping is " << ping << "us. Smoothed ping is " << _ping << std::endl;
				_pending_packets.remove(pack);
				break ;
			}
		}
		std::list<std::pair<uint16_t, int64_t>>::iterator it = _pending_packets.begin();
		// std::cout << "still in pending packets: ";
		while (it != _pending_packets.end()) {
			int diff = diff_wrap_around(packet.ack, it->first);
			if (diff > 32) { // packet lost
				it = _pending_packets.erase(it);
				// std::cout << "<LOSS diff " << diff << ", ack " << packet.ack << " it " << it->first  << '>' << std::endl;
				++_lost;
			} else if (diff < 0) {
				// std::cout << it->first << ' ';
				++it;
			} else if (packet.ack_bitfield & (1 << (diff - 1))) { // paquet has been aknoledged
				// std::cout << "<diff " << diff << ", ack " << packet.ack << " bitfield rmed " << it->first << '>' << std::endl;
				it = _pending_packets.erase(it);
			} else {
				// std::cout << it->first << ' ';
				++it;
			}
		}
		// std::cout << std::endl << "packet lost: " << _lost << ", pending " << _pending_packets.size() << ", packet sent: " << _sent << std::endl;
	} else if (_type == SOCKET::SERVER) {
		// update ack_bitfield for client's packet loss
		int diff = diff_wrap_around(packet.sequence, src->ack);
		src->bitfield <<= diff;
		// std::cout << "diff is " << diff;
		src->bitfield |= (1 << (diff - 1));
		// std::cout << ", bitfield is " << src->bitfield << std::endl;
	}

	src->ack = packet.sequence;
	packet.data[bytes] = '\0';
	memmove(data, packet.data, bytes);
	// std::cout << "received packet with header " << packet.protocol_id << " - " << packet.sequence << " - " << packet.ack << " - " << packet.ack_bitfield << std::endl;
	return (bytes);
}

int Socket::GetType( void )
{
	return (_type);
}

int Socket::GetId( Address & target )
{
	for (auto &c : _clients) {
		if (c.ip == target) {
			return (c.id);
		}
	}
	return (-1);
}

Address &Socket::GetServerAddress( void )
{
	return (_server_ip);
}

void *Socket::GetPingPtr( void )
{
	return (&_ping_ms);
}

void *Socket::GetPacketLostPtr( void )
{
	return (&_lost);
}

void *Socket::GetPacketSentPtr( void )
{
	return (&_sent);
}
