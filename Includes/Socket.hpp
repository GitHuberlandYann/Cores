#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Address.hpp"
# include <string>
# include <vector>
# include <list>

namespace SOCKET
{
	enum {
		SERVER,
		CLIENT
	};
}

// id used for this program, if other id found in packet, it is discarded
// for the program ./cores, id is "Core" (0x43 0x6F 0x72 0x65)
const unsigned int PROTOCOL_ID = 0x436F7265;
const int PACKET_SIZE_LIMIT = 1024;
const int CLIENT_SIZE_LIMIT = 8;
const uint16_t UINT16_HALF = 0x8000;

typedef struct s_packet {
	unsigned int protocol_id = 0;
	uint16_t sequence = 0;
	uint16_t ack = 0; // acknoledge
	unsigned int ack_bitfield = 0;
	char data[PACKET_SIZE_LIMIT];
}				t_packet;

typedef struct s_client {
	Address ip;
	unsigned int timeout = 0;
	int id = -1;
	uint16_t sequence = 0;
	uint16_t ack = -1;
	unsigned int bitfield = 0;
}				t_client;

class Socket
{
    public:

        Socket( int type = SOCKET::CLIENT );

        ~Socket( void );

        bool Open( unsigned short port = DEFAULT_PORT );
        void Close( void );
        bool IsOpen( void ) const;

        bool Send( const Address & destination, const void * data, int size );
        bool Send( const Address & destination, std::string str );
		void Broadcast( const void * data, int size );
		void Broadcast( std::string str );
        int Receive( Address & sender, void * data, int size );

		int GetType( void );
		int GetId( Address & target );
		Address &GetServerAddress( void );
		void *GetPingPtr( void );
		void *GetPacketLostPtr( void );
		void *GetPacketSentPtr( void );

    private:

        int _handle, _type, _ping;
		unsigned _sent, _lost;
		Address _server_ip;
		std::vector<t_client> _clients;
		// list of packets waiting for acknoledgment, with timestamps to compute ping
		std::list<std::pair<uint16_t, int64_t>> _pending_packets;
};

#endif
