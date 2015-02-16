#ifndef __PROCESSING_IMAGE_H__
#define __PROCESSING_IMAGE_H__

#include "Image.h"
#include "Debug.h"

class ProcessingImage
{
public:
	ProcessingImage(	unsigned	width,
						unsigned	height,
						unsigned	channels,
						unsigned	depth = 8 ) :
		_channels( channels ),
		_image( cvCreateImage( cvSize( width, height ), depth, channels ) )
	{
		assert( _image != NULL );
		assert( _image->imageData != NULL );
		assert( depth == 8 );
	}

	~ProcessingImage()
	{
		cvReleaseImage( & _image );
	}

	unsigned depth() const { return 8; }
	unsigned channels() const { return _channels; }
	unsigned height() const { return _image->height; }
	unsigned width() const { return _image->width; }

	void copyAndConvertToGray( const Image & source )
	{
		assert( source.channels() == 3 );
		assert( channels() == 1 );
		assert( source.width() == width() );
		assert( source.height() == height() );

		cvCvtColor( source.image(), _image, CV_BGR2GRAY );
	}

	void setRegionOfInterest( CvRect region )
	{
		cvSetImageROI( _image, region );
	}

	void clearRegionOfInterest()
	{
		cvResetImageROI( _image );
	}

	IplImage * image() const { return _image; }

	void findGoodFeatures( std::vector< CvPoint2D32f > & points )
	{
		assert( channels() == 1 );

		points.clear();
		static const unsigned MAXIMUM_POINTS = 1000;
		points.resize( MAXIMUM_POINTS );

		IplImage * eig = cvCreateImage( cvGetSize( _image ), 32, 1 );
		IplImage * temp = cvCreateImage( cvGetSize( _image ), 32, 1 );
		int count = MAXIMUM_POINTS;
		static const double QUALITY = 0.01;
		static const double MINIMUM_DISTANCE = 10;

		cvGoodFeaturesToTrack( _image, eig, temp, & points[ 0 ], & count,
							   QUALITY, MINIMUM_DISTANCE );

		assert( count <= (int) MAXIMUM_POINTS );
		points.resize( count );
		static const CvSize WINDOW = cvSize( 10, 10 );
		static const CvSize NO_ZERO_SIZE = cvSize( -1, -1 );
		static const CvTermCriteria TERMINATION_CRATERIA = 
			cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 );

		cvFindCornerSubPix( _image, & points[ 0 ], count,
							WINDOW, NO_ZERO_SIZE, TERMINATION_CRATERIA );

		cvReleaseImage( & eig );
		cvReleaseImage( & temp );
	}

private:
	const unsigned	_channels;
	IplImage *		_image;
};

#endif // __PROCESSING_IMAGE_H__
