#ifndef __FEATURES_DB_H__
#define __FEATURES_DB_H__

#include "opencv/cv.h"
#include <vector>
#include <map>

class FeaturesDB
{
public:
	typedef CvPoint2D32f Point;
	typedef std::vector< Point > PointVector;
	typedef std::vector< PointVector > FrameToPointsMap;

	void update( const FeaturesDB & other )
	{
		for ( unsigned i = 0 ; i < other._map.size() ; ++ i )
			if ( other._map[ i ].size() > 0 ) {
				if ( _map.size() <= i )
					_map.resize( i + 1 );
				_map[ i ] = other._map[ i ];
			}
	}

	void setPoints( unsigned frame, const PointVector & points )
	{
		if ( _map.size() < frame + 1 )
			_map.resize( frame + 1 );
		_map.at( frame ) = points;
	}

	void setInvertedPoints( unsigned frame, const PointVector & points, unsigned height )
	{
		if ( _map.size() < frame + 1 )
			_map.resize( frame + 1 );
		PointVector & result = _map.at( frame );
		result.resize( points.size() );
		for ( unsigned i = 0 ; i < points.size() ; ++ i ) {
			result[ i ].x = points[ i ].x;
			assert( points[ i ].y <= height - 1 );
			result[ i ].y = height - 1 - points[ i ].y;
		}
	}

	const PointVector & pointsOfFrame( unsigned frame )
	{
		static const PointVector EMPTY;

		if ( _map.size() <= frame )
			return EMPTY;

		return _map.at( frame );
	}

private:
	FrameToPointsMap	_map;
};

#endif // __FEATURES_DB_H__
