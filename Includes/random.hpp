#ifndef RANDOM_HPP
# define RANDOM_HPP

namespace Random {
	float randomFloat( unsigned &seed );
	int rangedNumber( unsigned &seed, int start, int end );
	std::vector<std::array<int, 2>> randomRGData( int sizeX, int sizeY );
}

#endif
