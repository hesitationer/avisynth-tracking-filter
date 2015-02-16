#ifndef __OBJECT_TRACKING_H__
#define __OBJECT_TRACKING_H__

#include <list>

#include "ProcessingImage.h"
#include "Debug.h"
#include "KeepPrevious.h"
#include "FeaturesDB.h"
#include "ProgressGUI.h"

class ObjectTracking
{
public:
	ObjectTracking(	PClip					clip,
					IScriptEnvironment *	scriptEnvironment,
					unsigned				selectionFrame,
					std::list< CvRect >		selections,
					const std::string &		outputFileName ):
		_clip( clip ),
		_scriptEnvironment( scriptEnvironment ),
		_firstPyramidAlreadyCalculated( false ),
		_output( outputFileName.c_str(), std::ios_base::app )
	{
		assert( ! selections.empty() );

		PVideoFrame frame = clip->GetFrame( selectionFrame, scriptEnvironment );
		Image frameImage( frame );
		IterationData a( frameImage.width(), frameImage.height() );
		IterationData b( frameImage.width(), frameImage.height() );
		KeepPreviousIterationData keepPrevious( a, b );

		findFeaturesInSelectionFrame( keepPrevious, frameImage, selections );
		_result.setPoints( selectionFrame, keepPrevious.current().points );
		unsigned initialPointsFound = keepPrevious.current().points.size();
		_output << "Selections at frame: " << selectionFrame <<
					", points found: " << initialPointsFound <<
					std::endl;
		for ( std::list< CvRect >::iterator i = selections.begin() ;
				i != selections.end() ; ++ i )
			_output << "Selection box: x: " << i->x <<
					", y: " << i->y <<
					", width: " << i->width <<
					", height: " << i->height <<
					std::endl;
		printPoints( selectionFrame, keepPrevious.current().points );

		std::vector< char > status( keepPrevious.current().points.size() );

		ProgressGUI progress;

		for( unsigned currentFrame = selectionFrame + 1; 
				currentFrame < (unsigned) clip->GetVideoInfo().num_frames ; ++ currentFrame ) {
			reportProgress( progress, currentFrame );
			trackIntoFrame( currentFrame, keepPrevious );
			if ( keepPrevious.current().points.size() < initialPointsFound / 2 ) {
				_output << "Lost tracking at frame " << currentFrame << std::endl;
				TRACE( "Lost tracking at frame " << currentFrame );
				std::ostringstream message;
				message << "Lost tracking at frame " << currentFrame;
				MessageBox( NULL, message.str().c_str(), "Object Tracking", MB_OK );
				return;
			}
			_result.setPoints( currentFrame, keepPrevious.current().points );
			printPoints( currentFrame, keepPrevious.current().points );
		}
		_output << "Finished tracking through the whole clip" << std::endl;
		MessageBox( NULL, "Finished tracking through the whole clip",
				"Object Tracking", MB_OK );
	}

	const FeaturesDB & result() const { return _result; }

private:
	struct IterationData
	{
		IterationData( unsigned width, unsigned height ) :
			gray( width, height, 1 ),
			pyramid( width, height, 1 )
		{
		}

		ProcessingImage gray;
		ProcessingImage pyramid;
		std::vector< CvPoint2D32f > points;
	}; 

	typedef KeepPrevious< IterationData > KeepPreviousIterationData;

	PClip					_clip;
	IScriptEnvironment *	_scriptEnvironment;
	bool					_firstPyramidAlreadyCalculated;
	std::vector< char > 	_iterationStatus;
	std::ofstream			_output;
	FeaturesDB				_result;

	static void reportProgress( ProgressGUI & progressGUI, unsigned frame )
	{
		std::ostringstream message;
		message << "Tracking at frame " << frame;
		progressGUI.setText( message.str() );
	}

	void calculateOpticalFlowIteration( KeepPreviousIterationData & keepPrevious )
	{
		static const CvSize WINDOW = cvSize( 10, 10 );
		static const int FOUR_PYRAMID_LEVELS = 3;
		static float * const DONT_REPORT_ERROR_DELTAS = NULL;
		static const CvTermCriteria TERMINATION_CRITERIA =
			cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 );

		if ( _iterationStatus.size() < keepPrevious.previous().points.size() )
			_iterationStatus.resize( keepPrevious.previous().points.size() );

		int flags = _firstPyramidAlreadyCalculated ? CV_LKFLOW_PYR_A_READY : 0;

		cvCalcOpticalFlowPyrLK(	keepPrevious.previous().gray.image(),
								keepPrevious.current().gray.image(),
								keepPrevious.previous().pyramid.image(),
								keepPrevious.current().pyramid.image(),
								& keepPrevious.previous().points[ 0 ],
								& keepPrevious.current().points[ 0 ],
								keepPrevious.previous().points.size(),
								WINDOW,
								FOUR_PYRAMID_LEVELS,
								& _iterationStatus[ 0 ],
								DONT_REPORT_ERROR_DELTAS,
								TERMINATION_CRITERIA,
								flags );

		_firstPyramidAlreadyCalculated = true;

		removeFeaturesThatFailedTracking( keepPrevious );
	}

	void removeFeaturesThatFailedTracking( KeepPreviousIterationData & keepPrevious )
	{
		std::vector< CvPoint2D32f > & points = keepPrevious.current().points;
		unsigned insertAt = 0;

		for ( unsigned i = 0 ; i < points.size() ; ++ i )
			if ( _iterationStatus[ i ] ) {
				points[ insertAt ] = points[ i ];
				++ insertAt;
			}

		points.resize( insertAt );
		keepPrevious.previous().points.resize( insertAt );
	}

	void shiftPointsIntoSelectionRegion(	std::vector< CvPoint2D32f > &	points,
											CvRect &						selection )
	{
		for ( std::vector< CvPoint2D32f >::iterator i = points.begin() ;
				i != points.end() ; ++ i ) {
			i->x += selection.x;
			i->y += selection.y;
		}
	}

	void trackIntoFrame( unsigned frameNumber, KeepPreviousIterationData & keepPrevious )
	{
		keepPrevious.switchCurrentAndPrevious();
		PVideoFrame frame = _clip->GetFrame( frameNumber, _scriptEnvironment );
		Image frameImage( frame );
		ProcessingImage & gray = keepPrevious.current().gray;
		gray.copyAndConvertToGray( frameImage );

		calculateOpticalFlowIteration( keepPrevious );

		TRACE( "At frame " << frameNumber << " found " <<
				keepPrevious.current().points.size() << " features" );
	}

	void findFeaturesInSelectionFrame(	KeepPreviousIterationData &		keepPrevious,
										Image &							frameImage,
										std::list< CvRect >				selection )
	{
		ProcessingImage & gray = keepPrevious.current().gray;
		gray.copyAndConvertToGray( frameImage );

		std::vector< CvPoint2D32f > singleSelectionFeatures;
		for ( std::list< CvRect >::iterator i = selection.begin() ;
				i != selection.end() ; ++ i ) {
			gray.setRegionOfInterest( * i );
			gray.findGoodFeatures( singleSelectionFeatures );
			gray.clearRegionOfInterest();
			shiftPointsIntoSelectionRegion( singleSelectionFeatures, * i );

			keepPrevious.current().points.insert(	keepPrevious.current().points.end(),
													singleSelectionFeatures.begin(),
				   									singleSelectionFeatures.end() );
		}

		keepPrevious.previous().points.resize( keepPrevious.current().points.size() );

		TRACE( "Points found: " << keepPrevious.current().points.size() );
	}

	void printPoints( unsigned frame, const std::vector< CvPoint2D32f > & points )
	{
		_output << "Frame: " << frame;
		for ( std::vector< CvPoint2D32f >::const_iterator i = points.begin() ;
				i != points.end() ; ++ i )
			_output << ", x: " << i->x <<
						", y: " << i->y;
		_output << std::endl;
	}
};

#endif // __OBJECT_TRACKING_H__
