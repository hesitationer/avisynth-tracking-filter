import trackeddb

class Tracked:
	def __init__( self, filename ):
		self._db = trackeddb.TrackedDB( filename )
		self.width = self._db.width
		self.height = self._db.height

	def centerOfFrame( self, frameNumber ):
		x = [ p[ 0 ] for p in self._db.db[ frameNumber ] ]
		y = [ p[ 1 ] for p in self._db.db[ frameNumber ] ]
		return self._averageWithoutPeaks( x ), self._averageWithoutPeaks( y )

	def frames( self ):
		return self._db.db.keys()

	def _averageWithoutPeaks( self, array ):
		array.sort()
		array = array[ len( array ) / 10 : - len( array ) / 10 ]
		return sum( array ) / len( array )

if __name__ == "__main__":
	import sys
	tracked = Tracked( sys.argv[ 1 ] )
	for frame in tracked._db.db.keys():
		print tracked.centerOfFrame( frame )
