import sys
import os
from os.path import *
import re

if len( sys.argv ) != 2:
	print "Usage: track.py <avisynth script.avs>"
	sys.exit( 1 )

script = sys.argv[ 1 ]
if not script.lower().endswith( ".avs" ):
	print "The argument provided is not an AVS file"
	sys.exit( 1 )

contents = open( script ).read()
directory = dirname( script )
scriptBasename = basename( script )[ : - len( ".avs" ) ] 
absoluteDirectory = abspath( script )
packageDir = dirname( dirname( sys.argv[ 0 ] ) )
plugin = abspath( join( packageDir, "filter.dll" ) )
if not exists( plugin ):
	print "Could not find %s" % plugin
	sys.stdin.readline()
	sys.exit( 1 )

trackingScript = join( directory, scriptBasename + "TrackingGUI.avs" )
trackingResult = join( directory, scriptBasename + "TrackingResult.csv" )

while True:
	print "Zoom out for large video? (1 no zoom, 2 is 2x...):"
	line = sys.stdin.readline().strip()
	if re.match( r"^\d+$", line ) and int( line ) != 0:
		break;
	print "Not a valid answer"
zoomOut = int( line )

f = open( trackingScript, "w" )
f.write( contents + "\n" )
f.write( "ConvertToRGB24()\n" )
f.write( 'LoadPlugin( "%s" )\n' % plugin )
f.write( 'TrackGUI( %s, "%s" )\n' % ( zoomOut, trackingResult ) )

print "Done!"
