#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "avisynth.h"

class Image
{
public:
	Image( unsigned width, unsigned height, unsigned depth = 8, unsigned channels = 3 ) :
		_image( cvCreateImage( cvSize( width, height ), depth, channels ) ),
		_const( false )
	{
		assert( _image != NULL );
		assert( _image->imageData != NULL );
		assert( depth == 8 );
		assert( channels == 3 );
		_image->origin = 1;
	}

	Image( const PVideoFrame & frame ) :
		_image( cvCreateImageHeader(
					cvSize( frame->GetRowSize() / FRAME_CHANNELS, frame->GetHeight() ),
									FRAME_DEPTH, FRAME_CHANNELS ) ),
		_const( true )
	{
		assert( _image != NULL );
		_image->origin = 1;
		cvSetData( _image, (void *) frame->GetReadPtr(), frame->GetPitch() );
		assert( _image->imageData != NULL );
		assert( _image->imageData == (char *) frame->GetReadPtr() );
	}

	~Image()
	{
		cvReleaseImage( & _image );
	}

	void copy( const Image & other ) const
	{
		assert( other.depth() == depth() );
		assert( other.channels() == channels() );
		assert( other.width() == width() );
		assert( other.height() == height() );
		assert( ! _const );

        cvCopy( other._image, _image, 0 );
	}

	void copyWithScaleDown( const Image & other )
	{
		assert( ! _const );
		assert( other.depth() == depth() );
		assert( other.channels() == channels() );
		unsigned scaleDown = other.width() / width();
		assert( width() * scaleDown == other.width() );
		assert( height() * scaleDown == other.height() );
		copyWithScaleDown( other._image->imageData, scaleDown );
	}

	unsigned depth() const { return 8; }
	unsigned channels() const { return 3; }
	unsigned height() const { return _image->height; }
	unsigned width() const { return _image->width; }

	void show( const char * windowName )
	{
		cvShowImage( windowName, _image );
	}

	void rectangle( CvRect rectangle, CvScalar color )
	{
		assert( ! _const );

		static const int THICKNESS = 2;

		cvRectangle( _image,
				cvPoint( rectangle.x, rectangle.y ),
				cvPoint( rectangle.x + rectangle.width, rectangle.y + rectangle.height ),
				color, THICKNESS );
	}

	IplImage * image() const { return _image; }

private:
	enum { FRAME_CHANNELS = 3, FRAME_DEPTH = 8 };

	IplImage *	_image;
	bool		_const;

	void copyWithScaleDown( const void * source, unsigned scaleDown )
	{
		assert( ! _const );

		const char * sourceCasted = (const char *) source;

		for ( unsigned y = 0 ; y < height() - 1 ; ++ y ) {
			char * targetRow = _image->imageData + y * width() * channels();
			const char * sourceRow = sourceCasted +
							y * scaleDown * width() * scaleDown * channels();
			for ( unsigned x = 0 ; x < width() ; ++ x )
				for ( unsigned c = 0 ; c < channels() ; ++ c )
					targetRow[ x * channels() + c ] =
						sourceRow[ x * scaleDown * channels() + c ];
		}
	}
};

#endif // __IMAGE_H__
