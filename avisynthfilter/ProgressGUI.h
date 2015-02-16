#ifndef __PROGRESS_GUI_H__
#define __PROGRESS_GUI_H__

#include <windows.h>
#include <stdexcept>
#include "Debug.h"

class ProgressGUI
{
public:
	ProgressGUI()
	{
		_hwnd = createWindow();
		ShowWindow( _hwnd, SW_SHOW );
		UpdateWindow( _hwnd );
	}

	~ProgressGUI()
	{
		DestroyWindow( _hwnd );
	}

	void setText( std::string text )
	{
		SendMessage( _hwnd, WM_SETTEXT, NULL, (LPARAM) text.c_str() );
		UpdateWindow( _hwnd );
	}

private:
	HWND _hwnd;

	static HWND createWindow()
	{
		HWND result = CreateWindow(	"Static",
									"Object Tracking Progress",
									0,
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									300,
									80,
									0,
									0,
									0,
									0 );

		if( result == NULL )
			throw std::runtime_error( "Unable to create window!" );

		return result;
	}
};

#endif // __PROGRESS_GUI_H__
