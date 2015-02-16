#ifndef __TRACK_GUI_FILTER_H__
#define __TRACK_GUI_FILTER_H__

#include "windows.h"
#include <memory>
#include <fstream>

#include "Selection.h"
#include "ObjectTracking.h"

class TrackGUIFilter : public GenericVideoFilter, public SelectionEventsInterface
{
public:
	TrackGUIFilter(	PClip					child,
					int						selectionScaleDown,
					const char *			outputFilename,
					IScriptEnvironment *	env ) :
		GenericVideoFilter( child ),
		_selectionScaleDown( selectionScaleDown ),
		_outputFilename( outputFilename ),
		_scriptEnvironment( env ),
		_lastFrame( UNSET ),
		_lastFrameShown( UNSET ),
		_width( 0 ),
		_height( 0 )
	{
		if ( vi.IsPlanar() )
			env->ThrowError( "TrackGUIFilter: input must be RGB24. "
					"Precede TrackGUIFilter() with ConvertRGB24()" );

		cvNamedWindow( windowName(), 1 );
		cvSetMouseCallback( windowName(), onMouseStatic, this );
	}

	~TrackGUIFilter()
	{
		cvDestroyWindow( windowName() );
	}

	PVideoFrame __stdcall GetFrame( int n, IScriptEnvironment * env )
	{
		_lastFrame = n;
		_selected.clear();

		PVideoFrame src = child->GetFrame( n, env );
		PVideoFrame dst = copy( src );
		drawFeaturesOnFrame( dst, n );
	
		return dst;
	}

private:
	enum { UNSET = (unsigned) -1, };

	const unsigned				_selectionScaleDown;
	const std::string			_outputFilename;
	IScriptEnvironment *		_scriptEnvironment;
	unsigned					_lastFrame;
	unsigned					_lastFrameShown;
	unsigned					_width;
	unsigned					_height;
	std::auto_ptr< Image >		_selectionImage;
	std::auto_ptr< Image >		_selectionImageOriginal;
	std::auto_ptr< Selection >	_selection;
	std::list< CvRect >			_selected;
	FeaturesDB					_features;

	static const char * windowName()
	{
		return "Object Tracking";
	}

	static void onMouseStatic( int event, int x, int y, int flags, void * param )
	{
		( (TrackGUIFilter *) param )->onMouse( event, x, y, flags );
	}
	
	void onMouse( int event, int x, int y, int flags )
	{
		if ( _lastFrameShown != _lastFrame && _lastFrame != UNSET ) {
			_lastFrameShown = _lastFrame;
			showLastFrame();
		}
		_selection->mouseInputInverted( event, x, y, flags );
	}

	static CvScalar red()
	{
		return CV_RGB( 255, 0, 0 );
	}

	void drawSelection( CvRect rectangle )
	{
		_selectionImage->copy( * _selectionImageOriginal );
		_selectionImage->rectangle( rectangle, red() );
		_selectionImage->show( windowName() );
	}

	void clearDrawing()
	{
		_selectionImage->copy( * _selectionImageOriginal );
		_selectionImage->show( windowName() );
	}

	void printSelection( CvRect rectangle )
	{
		std::ofstream out( _outputFilename.c_str(), std::ios_base::app );
		out <<	"Selection Frame: " << _lastFrame <<
				", x: " << rectangle.x <<
				", y: " << rectangle.y <<
				", width: " << rectangle.width <<
				", height: " << rectangle.height <<
				", frameWidth: " << _width <<
				", frameHeight: " << _height <<
				std::endl;
	}

	void selected( CvRect rectangle )
	{
		rectangle.x *= _selectionScaleDown;
		rectangle.y *= _selectionScaleDown;
		rectangle.width *= _selectionScaleDown;
		rectangle.height *= _selectionScaleDown;

		if ( rectangle.width < 24 || rectangle.height < 24 ) {
			MessageBox( NULL, "Selection Must be at least 24X24 Pixels", "Object Tracking", MB_ICONERROR );
			return;
		}

		printSelection( rectangle );

		int result = MessageBox( NULL, "Selection Added to Tracking. Start tracking now? (no allows more selections)", "Object Tracking", MB_YESNO );

		_selected.push_back( rectangle );
		if ( result == IDYES ) {
			try {
				ObjectTracking tracking(	child,
											_scriptEnvironment,
											_lastFrame,
											_selected,
											_outputFilename );
				_features.update( tracking.result() );
			} catch ( cv::Exception & e ) {
				std::ostringstream output;
				output << "OpenCV error: \"" << e.err << "\" (" << e.code <<
						") at " << e.func << ", " << e.file << ":" << e.line;
				MessageBox( NULL, output.str().c_str(), "Object Tracking Error", MB_ICONERROR );
			}
			_selected.clear();
		} 
	}

	void showLastFrame()
	{
		PVideoFrame frame = child->GetFrame( _lastFrame, _scriptEnvironment );
		Image frameImage( frame );

		_width = frameImage.width();
		_height = frameImage.height();

		if ( _selectionImage.get() == NULL )
			_selectionImage = std::auto_ptr< Image >( new Image(
						_width / _selectionScaleDown,
						_height / _selectionScaleDown ) );
		if ( _selectionImageOriginal.get() == NULL )
			_selectionImageOriginal = std::auto_ptr< Image >( new Image(
						_width / _selectionScaleDown,
						_height / _selectionScaleDown ) );
		if ( _selection.get() == NULL )
			_selection = std::auto_ptr< Selection >( new Selection(
						_width / _selectionScaleDown,
						_height / _selectionScaleDown,
					   	* this ) );
		
		_selectionImage->copyWithScaleDown( frameImage );
		_selectionImageOriginal->copy( * _selectionImage );
		_selectionImage->show( windowName() );
	}

	PVideoFrame copy( PVideoFrame src )
	{
		PVideoFrame result = _scriptEnvironment->NewVideoFrame( vi );
		assert( src->GetHeight() == result->GetHeight() );
		assert( src->GetPitch() == result->GetPitch() );
		assert( src->GetRowSize() == result->GetRowSize() );
		memcpy( result->GetWritePtr(), src->GetReadPtr(), src->GetPitch() * src->GetHeight() );
		return result;
	}

	void drawFeaturesOnFrame( PVideoFrame & dst, unsigned frameNumber )
	{
		const BYTE THREE_PIXELS[] = { 0, 255, 0, 0, 255, 0, 0, 255, 0 };
		BYTE * writePtr = dst->GetWritePtr();
		unsigned pitch = dst->GetPitch();
		const FeaturesDB::PointVector & points = _features.pointsOfFrame( frameNumber );

		for ( FeaturesDB::PointVector::const_iterator i = points.begin() ;
				i != points.end() ; ++ i )
			if ( i->x > 3 && i->x < _width - 3 &&
					i->y > 3 && i->y < _height - 3 ) {
				BYTE * pixel = writePtr + pitch * (unsigned) i->y + 3 * (unsigned) i->x;
				memcpy( pixel - 3 - pitch, THREE_PIXELS, sizeof( THREE_PIXELS ) );
				memcpy( pixel - 3, THREE_PIXELS, sizeof( THREE_PIXELS ) );
				memcpy( pixel - 3 + pitch, THREE_PIXELS, sizeof( THREE_PIXELS ) );
			}
	}
};

#endif // __TRACK_GUI_FILTER_H__
