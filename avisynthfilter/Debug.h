#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <windows.h>
#include <sstream>

#define TRACE( x ) do { \
		Debug debug; \
		debug << __FILE__ << ":" << __LINE__ << ": "; \
		debug << x; \
	} while( 0 )

class Debug : public std::ostringstream
{
public:
	Debug()
	{
		* this << (const char *) "SHLOMPI: ";
	}

	~Debug()
	{
		* this << (const char *) "\n";
		OutputDebugStringA( str().c_str() );
	}
};

#endif // __DEBUG_H__
