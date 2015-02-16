#ifndef __SELECTION_H__
#define __SELECTION_H__

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "Debug.h"

class SelectionEventsInterface
{
public:
	virtual ~SelectionEventsInterface() {}
	virtual void drawSelection( CvRect rectangle ) = 0;
	virtual void clearDrawing() = 0;
	virtual void selected( CvRect rectangle ) = 0;
};

class Selection
{
public:
	Selection(	unsigned					width,
				unsigned					height,
				SelectionEventsInterface &	selectionEvents ) :
		_width( width ),
		_height( height ),
		_selectionEvents( selectionEvents ),
		_selecting( false )
	{
	}

	void mouseInputInverted( int event, int x, int y, int flags )
	{
		return mouseInputNonInverted( event, x, _height - y, flags );
	}

	void mouseInputNonInverted( int event, int x, int y, int flags )
	{
		CvRect selection;
		if ( _selecting ) {
			selection.x = MIN( x, _origin.x );
			selection.y = MIN( y, _origin.y );
			selection.width = CV_IABS( x - _origin.x );
			selection.height = CV_IABS( y - _origin.y );

			selection.x = MAX( selection.x, 0 );
			selection.y = MAX( selection.y, 0 );
			selection.width = MIN( selection.width, (int) _width - selection.x );
			selection.height = MIN( selection.height, (int) _height - selection.y );
			_selectionEvents.drawSelection( selection );
		}

		switch( event ) {
			case CV_EVENT_LBUTTONDOWN:
				_selecting = true;
				_origin = cvPoint( x, y );
				break;
			case CV_EVENT_LBUTTONUP:
				_selecting = false;
				_selectionEvents.clearDrawing();
				if ( selection.width > 0 && selection.height > 0 )
					_selectionEvents.selected( selection );
				break;
		}
	}

private:
	const unsigned				_width;
	const unsigned				_height;
	SelectionEventsInterface &	_selectionEvents;
	bool						_selecting;
	CvPoint						_origin;
};

#endif // __SELECTION_H__
