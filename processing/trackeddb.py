import re

class TrackedDB:
	def __init__( self, filename ):
		self._readInfo( filename )
		self._readDB( filename )

	def _readInfo( self, filename ):
		content = open( filename ).read()
		all = re.findall( r"frameWidth: (\d+), frameHeight: (\d+)", content )
		first = all[ 0 ]
		for each in all:
			assert each == first
		self.width, self.height = [ int( i ) for i in first ]

	def _readDB( self, filename ):
		lines = open( filename ).readlines()
		result = {}
		for line in lines:
			if line.startswith( "Frame" ):
				split = line.split( ',' )
				frame = int( split[ 0 ][ len( "Frame: " ) : ] )
				split = split[ 1 : ]
				assert len( split ) % 2 == 0
				pixels = []
				for i in xrange( len( split ) / 2 ):
					assert split[ i * 2 ].strip().startswith( "x: " )
					assert split[ i * 2 + 1 ].strip().startswith( "y: " )
					x = int( float( split[ i * 2 ].strip()[ len( "x: " ) : ] ) )
					y = int( float( split[ i * 2 + 1 ].strip()[ len( "y: " ) : ] ) )
					pixels.append( ( x, y ) )
					result[ frame ] = pixels
		self.db = result

if __name__ == "__main__":
	import sys
	trackeddb = TrackedDB( sys.argv[ 1 ] )
