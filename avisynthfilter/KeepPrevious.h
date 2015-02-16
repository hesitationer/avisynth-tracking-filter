#ifndef __KEEP_PREVIOUS_H__
#define __KEEP_PREVIOUS_H__

template < typename T >
class KeepPrevious
{
public:
	KeepPrevious( T & current, T & previous ) :
		_a( current ),
		_b( previous ),
		_bIsPrevious( true )
	{
	}

	T & current() { return _bIsPrevious ? _a : _b; }
	T & previous() { return _bIsPrevious ? _b : _a; }
	void switchCurrentAndPrevious() { _bIsPrevious = ! _bIsPrevious; }

private:
	T & _a;
	T & _b;
	bool _bIsPrevious;
};

#endif // __KEEP_PREVIOUS_H__
