#include "Address.hpp"

Address::Address( void )
	: _address(LOCALHOST), _port(DEFAULT_PORT)
{

}

Address::Address( unsigned char a, unsigned char b,
	unsigned char c, unsigned char d, unsigned short port )
	: _port(port)
{
	_address = (a << 24) | (b << 16) | (c << 8) | d;
}

Address::Address( unsigned int address, unsigned short port )
	: _address(address), _port(port)
{

}

bool Address::operator==( const Address & other )
{
	return (_address == other.getAddress() && _port == other.getPort());
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

unsigned int Address::getAddress( void ) const
{
	return (_address);
}

unsigned char Address::getA( void ) const
{
	return ((_address >> 24) & 0xFF);
}

unsigned char Address::getB( void ) const
{
	return ((_address >> 16) & 0xFF);
}

unsigned char Address::getC( void ) const
{
	return ((_address >> 8) & 0xFF);
}

unsigned char Address::getD( void ) const
{
	return (_address & 0xFF);
}

unsigned short Address::getPort( void ) const
{
	return (_port);
}

// ************************************************************************** //
//                                Extern                                      //
// ************************************************************************** //

std::ostream &operator<<( std::ostream &out, const Address &a )
{
	out << static_cast<int>(a.getA()) << '.' << static_cast<int>(a.getB()) << '.'
		<< static_cast<int>(a.getC()) << '.' << static_cast<int>(a.getD()) << ':' << a.getPort();
	return (out);
}
