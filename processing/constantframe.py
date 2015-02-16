import tracked

class ConstantFrame:
	def __init__( self, trackeddbfilename, resolution, filenamePrefix ):
		self._filenamePrefix = filenamePrefix
		self.resolution = resolution
		self.tracked = tracked.Tracked( trackeddbfilename )
		self._writeOffsets()

	def xOffsetFilename( self ):
		return self._filenamePrefix + "XOffset.txt"

	def yOffsetFilename( self ):
		return self._filenamePrefix + "YOffset.txt"

	def _writeOffsets( self ):
		width, height = self.resolution
		maxX = self.tracked.width - width
		maxY = self.tracked.height - height
		xOut = open( self.xOffsetFilename(), "w" )
		xOut.write( "Type int\nDefault 0\n\n" )
		yOut = open( self.yOffsetFilename(), "w" )
		yOut.write( "Type int\nDefault 0\n\n" )
		for frame in self.tracked.frames():
			x, y = self.tracked.centerOfFrame( frame )
			y = self.tracked.height - y
			x -= width / 2
			y -= height / 2
			x = min( maxX, x )
			y = min( maxY, y )
			x = max( x, 0 )
			y = max( y, 0 )
			xOut.write( "%d %d\n" % ( frame, x ) )
			yOut.write( "%d %d\n" % ( frame, y ) )
		xOut.close()
		yOut.close()

if __name__ == "__main__":
	import sys
	ConstantFrame( * sys.argv[ 1 : ] )
