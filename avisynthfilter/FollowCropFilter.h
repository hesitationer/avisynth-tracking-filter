#ifndef __FOLLOW_CROP_FILTER_H__
#define __FOLLOW_CROP_FILTER_H__

#include <string>
#include "avisynth.h"

class FollowCropFilter : public GenericVideoFilter
{
public:
	FollowCropFilter(	PClip					child,
						int						width,
						int						height,
						const char *			xVariableName,
						const char *			yVariableName,
						IScriptEnvironment *	env ):
		GenericVideoFilter( child ),
		_width( width ),
		_height( height ),
		_xVariableName( xVariableName ),
		_yVariableName( yVariableName )
	{
		if ( _width < 0 )
			env->ThrowError( "FollowCrop: width is negative" );
		if ( _height < 0 )
			env->ThrowError( "FollowCrop: height is negative" );
		if ( _width > vi.width )
			env->ThrowError( "FollowCrop: width larger than source" );
		if ( _height > vi.height )
			env->ThrowError( "FollowCrop: height larget than source" );
		if ( vi.IsYUV() ) {
			if ( _width % 2 != 0 )
				env->ThrowError( "FollowCrop: YUV format, width must be even" );
			if ( vi.IsYV12() )
				if ( _height % 2 != 0 )
					env->ThrowError( "FollowCrop: YUV format, height must be even" );
		}
		vi.width = _width;
		vi.height = _height;
	}

	PVideoFrame __stdcall GetFrame( int n, IScriptEnvironment * env )
	{
		PVideoFrame frame = child->GetFrame(n, env);
		unsigned x = getX( env );
		unsigned y = getY( env );
		if ( vi.IsRGB() )
			y = frame->GetHeight() - vi.height - y;
		unsigned leftBytes = vi.BytesFromPixels( x );

		const BYTE * srcpU = NULL;
		const BYTE * srcpV = NULL;

		if ( vi.IsYUV() ) {
			if ( x % 2 != 0 )
				x -= 1;
			if (vi.IsYV12()) {
				if ( y % 2 != 0 )
					y -= 1;
				return env->SubframePlanar(
							frame,
							y * frame->GetPitch() + leftBytes,
							frame->GetPitch(),
							vi.RowSize(),
							vi.height,
							( y >> 1 ) * frame->GetPitch( PLANAR_U ) + ( leftBytes >> 1 ),
							( y >> 1 ) * frame->GetPitch( PLANAR_V ) + ( leftBytes >> 1 ),
							frame->GetPitch( PLANAR_U ) );
			} else
				return env->SubframePlanar(	frame,
											y * frame->GetPitch() + leftBytes,
											frame->GetPitch(),
											vi.RowSize(),
											vi.height,
											y * frame->GetPitch( PLANAR_U ) + leftBytes,
											y * frame->GetPitch( PLANAR_V ) + leftBytes,
											frame->GetPitch( PLANAR_U ) );
		} else
			return env->Subframe(	frame,
									y * frame->GetPitch() + leftBytes,
									frame->GetPitch(),
									vi.RowSize(),
									vi.height );
	}

private:
	PClip				_child;
	const int			_width;
	const int			_height;
	const std::string	_xVariableName;
	const std::string	_yVariableName;

	unsigned getX( IScriptEnvironment * env )
	{
		try {
			AVSValue cv = env->GetVar( _xVariableName.c_str() );
			if ( ! cv.IsInt() )
				env->ThrowError( "xVariableName is not int" );
			int result = cv.AsInt();
			if ( result < 0 )
				env->ThrowError( "xVariableName is negative" );
			return (unsigned) result;
		} catch ( IScriptEnvironment::NotFound & ) {
			env->ThrowError( "xVariableName is not defined" );
		}
		return 0;
	}

	unsigned getY( IScriptEnvironment * env )
	{
		try {
			AVSValue cv = env->GetVar( _yVariableName.c_str() );
			if ( ! cv.IsInt() )
				env->ThrowError( "yVariableName is not int" );
			int result = cv.AsInt();
			if ( result < 0 )
				env->ThrowError( "yVariableName is negative" );
			return (unsigned) result;
		} catch ( IScriptEnvironment::NotFound & ) {
			env->ThrowError( "yVariableName is not defined" );
		}
		return 0;
	}
};

#endif // __FOLLOW_CROP_FILTER_H__
