#ifndef ADDRESS_HPP
# define ADDRESS_HPP

# include <iostream>

// 127.0.0.1
const unsigned LOCALHOST = (127 << 24) + 1;
const unsigned short DEFAULT_PORT = 31415;

class Address
{
	public:

		Address( void );

		Address( unsigned char a, 
				unsigned char b, 
				unsigned char c, 
				unsigned char d, 
				unsigned short port = DEFAULT_PORT );

		Address( unsigned int address, 
				unsigned short port = DEFAULT_PORT );
		
		bool operator==( const Address & other );

		unsigned int getAddress( void ) const;

		unsigned char getA( void ) const;
		unsigned char getB( void ) const;
		unsigned char getC( void ) const;
		unsigned char getD( void ) const;

		unsigned short getPort( void ) const;

	private:

		unsigned int _address;
		unsigned short _port;
};

std::ostream &operator<<( std::ostream &out, const Address &a );

#endif
